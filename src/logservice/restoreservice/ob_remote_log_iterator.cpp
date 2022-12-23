/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#define USING_LOG_PREFIX CLOG
#include "ob_remote_log_iterator.h"
#include <cstdint>
#include "lib/ob_define.h"
#include "lib/ob_errno.h"
#include "lib/utility/ob_macro_utils.h"
#include "lib/net/ob_addr.h"
#include "lib/profile/ob_trace_id.h"
#include "lib/utility/ob_tracepoint.h"                  // EventTable
#include "lib/restore/ob_storage.h"                     // is_io_error
#include "logservice/ob_log_service.h"
#include "logservice/palf/log_group_entry.h"            // LogGroupEntry
#include "ob_remote_log_source.h"
#include "share/restore/ob_log_archive_source.h"
#include "share/backup/ob_backup_struct.h"
#include "share/rc/ob_tenant_base.h"

namespace oceanbase
{
namespace logservice
{
using namespace oceanbase::palf;
using namespace oceanbase::share;

// ===================================== ObRemoteLogIterator ============================ //
ObRemoteLogIterator::ObRemoteLogIterator(GetSourceFunc &get_source_func,
    UpdateSourceFunc &update_source_func,
    RefreshStorageInfoFunc &refresh_storage_info_func) :
  inited_(false),
  tenant_id_(OB_INVALID_TENANT_ID),
  id_(),
  start_lsn_(),
  cur_lsn_(),
  cur_scn_(),
  end_lsn_(),
  source_guard_(),
  data_buffer_(),
  gen_(NULL),
  get_source_func_(get_source_func),
  update_source_func_(update_source_func),
  refresh_storage_info_func_(refresh_storage_info_func)
{}

ObRemoteLogIterator::~ObRemoteLogIterator()
{
  int ret = OB_SUCCESS;
  if (OB_FAIL(update_source_func_(id_, source_guard_.get_source()))) {
    LOG_WARN("update source failed", K(ret), KPC(this));
  }

  inited_ = false;
  if (NULL != gen_) {
    MTL_DELETE(RemoteDataGenerator, "ResDataGen",gen_);
    gen_ = NULL;
  }
  id_.reset();
  start_lsn_.reset();
  cur_lsn_.reset();
  cur_scn_.reset();
  end_lsn_.reset();
  data_buffer_.reset();
}

int ObRemoteLogIterator::init(const uint64_t tenant_id,
    const ObLSID &id,
    const SCN &pre_scn,
    const LSN &start_lsn,
    const LSN &end_lsn)
{
  int ret = OB_SUCCESS;
  ObRemoteLogParent *source = NULL;
  if (OB_UNLIKELY(inited_)) {
    ret = OB_INIT_TWICE;
    LOG_WARN("ObRemoteLogIterator already init", K(ret), K(inited_), K(id_));
  } else if (OB_UNLIKELY(OB_INVALID_TENANT_ID == tenant_id
        || ! id.is_valid()
        || ! start_lsn.is_valid()
        || (end_lsn.is_valid() && end_lsn <= start_lsn))) {
    ret = OB_INVALID_ARGUMENT;
    LOG_WARN("invalid argument", K(ret), K(tenant_id), K(id), K(start_lsn), K(end_lsn));
  } else if (OB_FAIL(get_source_func_(id, source_guard_))) {
    LOG_WARN("get source failed", K(ret), K(id));
  } else if (OB_ISNULL(source = source_guard_.get_source())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("source is NULL", K(ret), K(id));
  } else if (OB_UNLIKELY(! share::is_location_log_source_type(source->get_source_type())
        && ! share::is_raw_path_log_source_type(source->get_source_type()))) {
    ret = OB_NOT_SUPPORTED;
    LOG_WARN("source type not support", K(ret), K(id), KPC(source));
  } else {
    tenant_id_ = tenant_id;
    id_ = id;
    start_lsn_ = start_lsn;
    end_lsn_ = end_lsn;
    ret = build_data_generator_(pre_scn, source, refresh_storage_info_func_);
    LOG_INFO("ObRemoteLogIterator init", K(ret), K(tenant_id), K(id), K(pre_scn), K(start_lsn), K(end_lsn));
  }

  if (OB_SUCC(ret)) {
    inited_ = true;
  }
  return ret;
}

int ObRemoteLogIterator::next(LogGroupEntry &entry, LSN &lsn, char *&buf, int64_t &buf_size)
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(! inited_)) {
    ret = OB_NOT_INIT;
    LOG_WARN("ObRemoteLogIterator not init", K(ret), K(inited_));
  } else {
    ret = next_entry_(entry, lsn, buf, buf_size);
  }

#ifdef ERRSIM
  if (OB_SUCC(ret)) {
    ret = OB_E(EventTable::EN_RESTORE_LOG_FROM_SOURCE_FAILED) OB_SUCCESS;
  }
#endif
  return ret;
}

int ObRemoteLogIterator::get_cur_lsn_scn(LSN &lsn, SCN &scn) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(! inited_)) {
    ret = OB_NOT_INIT;
  } else {
    lsn = cur_lsn_;
    scn = cur_scn_;
  }
  return ret;
}

int ObRemoteLogIterator::build_data_generator_(const SCN &pre_scn,
    ObRemoteLogParent *source,
    RefreshStorageInfoFunc &refresh_storage_info_func)
{
  int ret = OB_SUCCESS;
  const share::ObLogArchiveSourceType &type = source->get_source_type();
  if (is_service_log_source_type(type)) {
    ObRemoteSerivceParent *service_source = static_cast<ObRemoteSerivceParent *>(source);
    ret = build_service_data_generator_(service_source);
  } else if (is_raw_path_log_source_type(type)) {
    ObRemoteRawPathParent *dest_source = static_cast<ObRemoteRawPathParent *>(source);
    ret = build_dest_data_generator_(pre_scn, dest_source);
  } else if (is_location_log_source_type(type)) {
    ObRemoteLocationParent *location_source = static_cast<ObRemoteLocationParent *>(source);
    ret = build_location_data_generator_(pre_scn, location_source, refresh_storage_info_func);
  } else {
    ret = OB_NOT_SUPPORTED;
  }
  if (OB_SUCC(ret)) {
    LOG_INFO("remote iterator init succ", KPC(this));
  }
  return ret;
}

int ObRemoteLogIterator::build_service_data_generator_(ObRemoteSerivceParent *source)
{
  int ret = OB_SUCCESS;
  SCN end_scn;
  ObAddr server;
  source->get(server, end_scn);
  gen_ = MTL_NEW(ServiceDataGenerator, "ResDataGen", tenant_id_, id_, start_lsn_, end_lsn_, end_scn, server);
  if (OB_ISNULL(gen_)) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("alloc service data generator failed", K(ret), KPC(this));
  }
  return ret;
}

int ObRemoteLogIterator::build_dest_data_generator_(const SCN &pre_scn, ObRemoteRawPathParent *source)
{
  int ret = OB_SUCCESS;
  UNUSED(pre_scn);
  logservice::DirArray array;
  SCN end_scn;
  int64_t piece_index = 0;
  int64_t min_file_id = 0;
  int64_t max_file_id = 0;
  source->get(array, end_scn);
  source->get_locate_info(piece_index, min_file_id, max_file_id);
  gen_ = MTL_NEW(RawPathDataGenerator, "ResDataGen", tenant_id_, id_, start_lsn_, end_lsn_,
      array, end_scn, piece_index, min_file_id, max_file_id);
  if (OB_ISNULL(gen_)) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("alloc dest data generator failed", K(ret), KPC(this));
  }
  return ret;
}

int ObRemoteLogIterator::build_location_data_generator_(const SCN &pre_scn,
    ObRemoteLocationParent *source,
    const std::function<int(share::ObBackupDest &dest)> &refresh_storage_info_func)
{
  int ret = OB_SUCCESS;
  UNUSED(refresh_storage_info_func);
  SCN end_scn;
  share::ObBackupDest *dest = NULL;
  ObLogArchivePieceContext *piece_context = NULL;
  source->get(dest, piece_context, end_scn);
  gen_ = MTL_NEW(LocationDataGenerator, "ResDataGen", tenant_id_, pre_scn,
      id_, start_lsn_, end_lsn_, end_scn, dest, piece_context);
  if (OB_ISNULL(gen_)) {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    LOG_WARN("alloc location data generator failed", K(ret), KPC(this));
  }
  return ret;
}

int ObRemoteLogIterator::next_entry_(LogGroupEntry &entry, LSN &lsn, char *&buf, int64_t &buf_size)
{
  int ret = OB_SUCCESS;
  if (data_buffer_.is_empty() && !data_buffer_.is_valid()) {
    ret = prepare_buf_(data_buffer_);
  }

  if (OB_SUCC(ret)) {
    ret = get_entry_(entry, lsn, buf, buf_size);
    if (OB_ITER_END == ret) {
      update_data_gen_max_lsn_();
    }
  }

  if (OB_SUCC(ret)) {
    cur_lsn_ = lsn + entry.get_serialize_size();
    cur_scn_ = entry.get_header().get_max_scn();
  }

  if (OB_FAIL(ret) && OB_ITER_END != ret && ! is_io_error(ret)) {
    mark_source_error_(ret);
  }
  return ret;
}

int ObRemoteLogIterator::prepare_buf_(RemoteDataBuffer &buffer)
{
  int ret = OB_SUCCESS;
  if (OB_ISNULL(gen_)) {
    ret = OB_ERR_UNEXPECTED;
  } else {
    ret = gen_->next_buffer(buffer);
  }
  return ret;
}

int ObRemoteLogIterator::get_entry_(LogGroupEntry &entry, LSN &lsn, char *&buf, int64_t &buf_size)
{
  return data_buffer_.next(entry, lsn, buf, buf_size);
}

void ObRemoteLogIterator::update_data_gen_max_lsn_()
{
  if (NULL != gen_ && cur_lsn_.is_valid()) {
    gen_->update_max_lsn(cur_lsn_);
  }
}

void ObRemoteLogIterator::mark_source_error_(const int ret_code)
{
  int ret = OB_SUCCESS;
  ObRemoteLogParent *source = NULL;
  if (OB_ISNULL(source = source_guard_.get_source())) {
    ret = OB_ERR_UNEXPECTED;
    LOG_WARN("source is NULL", K(ret), K(ret_code), K(id_));
  } else {
    source->mark_error(*ObCurTraceId::get_trace_id(), ret_code);
  }
}

bool ObRemoteLogIterator::is_retry_ret_(const bool ret_code) const
{
  return OB_ALLOCATE_MEMORY_FAILED == ret_code
    || is_io_error(ret_code);
}
} // namespace logservice
} // namespace oceanbase
