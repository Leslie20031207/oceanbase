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

#ifndef OCEABASE_STORAGE_RPC
#define OCEABASE_STORAGE_RPC

#include "lib/net/ob_addr.h"
#include "lib/utility/ob_unify_serialize.h"
#include "rpc/obrpc/ob_rpc_packet.h"
#include "rpc/obrpc/ob_rpc_proxy.h"
#include "rpc/obrpc/ob_rpc_processor.h"
#include "rpc/obrpc/ob_rpc_result_code.h"
#include "common/ob_member.h"
#include "storage/ob_storage_struct.h"
#include "observer/ob_server_struct.h"
#include "storage/ob_storage_schema.h"
#include "storage/high_availability/ob_storage_ha_struct.h"
#include "storage/blocksstable/ob_sstable_meta.h"
#include "storage/ls/ob_ls_meta_package.h"
#include "tablet/ob_tablet_meta.h"
#include "share/restore/ob_ls_restore_status.h"
#include "share/transfer/ob_transfer_info.h"
#include "storage/lob/ob_lob_rpc_struct.h"
#include "storage/blocksstable/ob_logic_macro_id.h"

namespace oceanbase
{
namespace observer
{
struct ObGlobalContext;
}

namespace storage
{
class ObLogStreamService;
}

namespace obrpc
{

//TODO(yanfeng) need use tenant module replace it in 4.1, currently use 509 tenant

struct ObCopyMacroBlockArg
{
  OB_UNIS_VERSION(2);
public:
  ObCopyMacroBlockArg();
  virtual ~ObCopyMacroBlockArg() {}
  void reset();
  bool is_valid() const;
  TO_STRING_KV(K_(logic_macro_block_id));
  blocksstable::ObLogicMacroBlockId logic_macro_block_id_;
};

struct ObCopyMacroBlockListArg
{
  OB_UNIS_VERSION(2);
public:
  ObCopyMacroBlockListArg();
  virtual ~ObCopyMacroBlockListArg() {}

  void reset();
  bool is_valid() const;
  int assign(const ObCopyMacroBlockListArg &arg);

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(table_key), "arg_count", arg_list_.count());
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  storage::ObITable::TableKey table_key_;
  common::ObSArray<ObCopyMacroBlockArg> arg_list_;
};

struct ObCopyMacroBlockRangeArg final
{
  OB_UNIS_VERSION(2);
public:
  ObCopyMacroBlockRangeArg();
  ~ObCopyMacroBlockRangeArg() {}

  void reset();
  bool is_valid() const;
  int assign(const ObCopyMacroBlockRangeArg &arg);
  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(table_key), K_(data_version), K_(backfill_tx_scn), K_(copy_macro_range_info));

  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  storage::ObITable::TableKey table_key_;
  int64_t data_version_;
  share::SCN backfill_tx_scn_;
  storage::ObCopyMacroRangeInfo copy_macro_range_info_;
  bool need_check_seq_;
  int64_t ls_rebuild_seq_;
  DISALLOW_COPY_AND_ASSIGN(ObCopyMacroBlockRangeArg);
};

struct ObCopyMacroBlockHeader
{
  OB_UNIS_VERSION(2);
public:
  ObCopyMacroBlockHeader();
  virtual ~ObCopyMacroBlockHeader() {}
  void reset();
  bool is_valid() const;

  TO_STRING_KV(K_(is_reuse_macro_block), K_(occupy_size));
  bool is_reuse_macro_block_;
  int64_t occupy_size_;
};

struct ObCopyTabletInfoArg
{
  OB_UNIS_VERSION(2);
public:
  ObCopyTabletInfoArg();
  virtual ~ObCopyTabletInfoArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(tablet_id_list), K_(need_check_seq),
      K_(ls_rebuild_seq), K_(is_only_copy_major), K_(version));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  common::ObSArray<common::ObTabletID> tablet_id_list_;
  bool need_check_seq_;
  int64_t ls_rebuild_seq_;
  bool is_only_copy_major_;
  uint64_t version_;
};

struct ObCopyTabletInfo
{
  OB_UNIS_VERSION(2);
public:
  ObCopyTabletInfo();
  virtual ~ObCopyTabletInfo() {}
  void reset();
  int assign(const ObCopyTabletInfo &info);
  bool is_valid() const;
  TO_STRING_KV(K_(tablet_id), K_(status), K_(param), K_(data_size), K_(version));

  common::ObTabletID tablet_id_;
  storage::ObCopyTabletStatus::STATUS status_;
  storage::ObMigrationTabletParam param_;
  int64_t data_size_; //need copy ssttablet size
  uint64_t version_;
};

struct ObCopyTabletSSTableInfoArg final
{
  OB_UNIS_VERSION(2);
public:
  ObCopyTabletSSTableInfoArg();
  ~ObCopyTabletSSTableInfoArg();
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(tablet_id), K_(max_major_sstable_snapshot), K_(minor_sstable_scn_range),
      K_(ddl_sstable_scn_range));

  common::ObTabletID tablet_id_;
  int64_t max_major_sstable_snapshot_;
  share::ObScnRange minor_sstable_scn_range_;
  share::ObScnRange ddl_sstable_scn_range_;
};

struct ObCopyTabletsSSTableInfoArg final
{
  OB_UNIS_VERSION(2);
public:
  ObCopyTabletsSSTableInfoArg();
  ~ObCopyTabletsSSTableInfoArg();
  bool is_valid() const;
  void reset();
  int assign(const ObCopyTabletsSSTableInfoArg &arg);

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(need_check_seq),
      K_(ls_rebuild_seq), K_(is_only_copy_major), K_(tablet_sstable_info_arg_list),
      K_(version));

  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  bool need_check_seq_;
  int64_t ls_rebuild_seq_;
  bool is_only_copy_major_;
  common::ObSArray<ObCopyTabletSSTableInfoArg> tablet_sstable_info_arg_list_;
  uint64_t version_;
  DISALLOW_COPY_AND_ASSIGN(ObCopyTabletsSSTableInfoArg);
};

struct ObCopyTabletSSTableInfo
{
  OB_UNIS_VERSION(2);
public:
  ObCopyTabletSSTableInfo();
  virtual ~ObCopyTabletSSTableInfo() {}
  void reset();
  int assign(const ObCopyTabletSSTableInfo &info);
  bool is_valid() const;
  TO_STRING_KV(K_(tablet_id), K_(table_key), K_(param));

  common::ObTabletID tablet_id_;
  storage::ObITable::TableKey table_key_;
  blocksstable::ObMigrationSSTableParam param_;
};

struct ObCopyLSInfoArg
{
  OB_UNIS_VERSION(2);
public:
  ObCopyLSInfoArg();
  virtual ~ObCopyLSInfoArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  uint64_t version_;
};

struct ObCopyLSInfo
{
  OB_UNIS_VERSION(2);
public:
  ObCopyLSInfo();
  virtual ~ObCopyLSInfo() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(ls_meta_package), K_(tablet_id_array), K_(is_log_sync), K_(version));
  storage::ObLSMetaPackage ls_meta_package_;
  common::ObSArray<common::ObTabletID> tablet_id_array_;
  bool is_log_sync_;
  uint64_t version_;
};

struct ObFetchLSMetaInfoArg
{
  OB_UNIS_VERSION(2);
public:
  ObFetchLSMetaInfoArg();
  virtual ~ObFetchLSMetaInfoArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(version));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  uint64_t version_;
};

struct ObFetchLSMetaInfoResp
{
  OB_UNIS_VERSION(2);
public:
  ObFetchLSMetaInfoResp();
  virtual ~ObFetchLSMetaInfoResp() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(ls_meta_package), K_(has_transfer_table), K_(version));
  storage::ObLSMetaPackage ls_meta_package_;
  uint64_t version_;
  bool has_transfer_table_;
};

struct ObFetchLSMemberListArg
{
  OB_UNIS_VERSION(2);
public:
  ObFetchLSMemberListArg();
  virtual ~ObFetchLSMemberListArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
};

struct ObFetchLSMemberListInfo
{
  OB_UNIS_VERSION(2);
public:
  ObFetchLSMemberListInfo();
  virtual ~ObFetchLSMemberListInfo() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(member_list));
  common::ObMemberList member_list_;
};

struct ObCopySSTableMacroRangeInfoArg final
{
  OB_UNIS_VERSION(2);
public:
  ObCopySSTableMacroRangeInfoArg();
  ~ObCopySSTableMacroRangeInfoArg();
  bool is_valid() const;
  void reset();
  int assign(const ObCopySSTableMacroRangeInfoArg &arg);

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(tablet_id), K_(copy_table_key_array), K_(macro_range_max_marco_count));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  common::ObTabletID tablet_id_;
  common::ObSArray<ObITable::TableKey> copy_table_key_array_;
  int64_t macro_range_max_marco_count_;
  bool need_check_seq_;
  int64_t ls_rebuild_seq_;
  DISALLOW_COPY_AND_ASSIGN(ObCopySSTableMacroRangeInfoArg);
};

struct ObCopySSTableMacroRangeInfoHeader final
{
  OB_UNIS_VERSION(2);
public:
  ObCopySSTableMacroRangeInfoHeader();
  ~ObCopySSTableMacroRangeInfoHeader();
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(copy_table_key), K_(macro_range_count));

  ObITable::TableKey copy_table_key_;
  int64_t macro_range_count_;
};

struct ObCopyTabletSSTableHeader final
{
  OB_UNIS_VERSION(2);
public:
  ObCopyTabletSSTableHeader();
  ~ObCopyTabletSSTableHeader() {}
  void reset();
  bool is_valid() const;
  TO_STRING_KV(K_(tablet_id), K_(status), K_(sstable_count), K_(tablet_meta), K_(version));

  common::ObTabletID tablet_id_;
  storage::ObCopyTabletStatus::STATUS status_;
  int64_t sstable_count_;
  ObMigrationTabletParam tablet_meta_;
  uint64_t version_;
};

// Leader notify follower to restore some tablets.
struct ObNotifyRestoreTabletsArg
{
  OB_UNIS_VERSION(2);
public:
  ObNotifyRestoreTabletsArg();
  virtual ~ObNotifyRestoreTabletsArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(tablet_id_array), K_(restore_status), K_(leader_proposal_id));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  common::ObSArray<common::ObTabletID> tablet_id_array_;
  share::ObLSRestoreStatus restore_status_; // indicate the type of data to restore
  int64_t leader_proposal_id_;
};

struct ObNotifyRestoreTabletsResp
{
  OB_UNIS_VERSION(2);
public:
  ObNotifyRestoreTabletsResp();
  virtual ~ObNotifyRestoreTabletsResp() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(restore_status));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  share::ObLSRestoreStatus restore_status_; // restore status
};


struct ObInquireRestoreResp
{
  OB_UNIS_VERSION(2);
public:
  ObInquireRestoreResp();
  virtual ~ObInquireRestoreResp() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(is_leader), K_(restore_status));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  bool is_leader_;
  share::ObLSRestoreStatus restore_status_; // leader restore status
};

struct ObInquireRestoreArg
{
  OB_UNIS_VERSION(2);
public:
  ObInquireRestoreArg();
  virtual ~ObInquireRestoreArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(restore_status));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  share::ObLSRestoreStatus restore_status_; // restore status
};

struct ObRestoreUpdateLSMetaArg
{
  OB_UNIS_VERSION(2);
public:
  ObRestoreUpdateLSMetaArg();
  virtual ~ObRestoreUpdateLSMetaArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_meta_package));
  uint64_t tenant_id_;
  storage::ObLSMetaPackage ls_meta_package_;
};

//transfer
struct ObCheckSrcTransferTabletsArg final
{
  OB_UNIS_VERSION(1);
public:
  ObCheckSrcTransferTabletsArg();
  ~ObCheckSrcTransferTabletsArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(src_ls_id), K_(tablet_info_array));
  uint64_t tenant_id_;
  share::ObLSID src_ls_id_;
  common::ObSArray<share::ObTransferTabletInfo> tablet_info_array_;
private:
  DISALLOW_COPY_AND_ASSIGN(ObCheckSrcTransferTabletsArg);
};

struct ObGetLSActiveTransCountArg final
{
  OB_UNIS_VERSION(1);
public:
  ObGetLSActiveTransCountArg();
  ~ObGetLSActiveTransCountArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(src_ls_id));
  uint64_t tenant_id_;
  share::ObLSID src_ls_id_;
};

struct ObGetLSActiveTransCountRes final
{
  OB_UNIS_VERSION(1);
public:
  ObGetLSActiveTransCountRes();
  ~ObGetLSActiveTransCountRes() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(active_trans_count));
  int64_t active_trans_count_;
};

struct ObGetTransferStartScnArg final
{
  OB_UNIS_VERSION(1);
public:
  ObGetTransferStartScnArg();
  ~ObGetTransferStartScnArg() {}
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(tenant_id), K_(src_ls_id), K_(tablet_list));
  uint64_t tenant_id_;
  share::ObLSID src_ls_id_;
  common::ObSArray<share::ObTransferTabletInfo> tablet_list_;
};

struct ObGetTransferStartScnRes final
{
  OB_UNIS_VERSION(1);
public:
  ObGetTransferStartScnRes();
  ~ObGetTransferStartScnRes() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(start_scn));
  share::SCN start_scn_;
};

struct ObTransferTabletInfoArg final
{
  OB_UNIS_VERSION(1);
public:
  ObTransferTabletInfoArg();
  ~ObTransferTabletInfoArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(src_ls_id), K_(dest_ls_id), K_(tablet_list));
  uint64_t tenant_id_;
  share::ObLSID src_ls_id_;
  share::ObLSID dest_ls_id_;
  common::ObSArray<share::ObTransferTabletInfo> tablet_list_;
private:
  DISALLOW_COPY_AND_ASSIGN(ObTransferTabletInfoArg);
};

struct ObFetchLSReplayScnArg final
{
  OB_UNIS_VERSION(1);
public:
  ObFetchLSReplayScnArg();
  ~ObFetchLSReplayScnArg() {}
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(tenant_id), K_(ls_id));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
};

struct ObFetchLSReplayScnRes final
{
  OB_UNIS_VERSION(1);
public:
  ObFetchLSReplayScnRes();
  ~ObFetchLSReplayScnRes() {}
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(replay_scn));
  share::SCN replay_scn_;
};

struct ObCheckTransferTabletBackfillArg final
{
  OB_UNIS_VERSION(1);
public:
  ObCheckTransferTabletBackfillArg();
  ~ObCheckTransferTabletBackfillArg() {}
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(tablet_list));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  common::ObSArray<share::ObTransferTabletInfo> tablet_list_;
private:
  DISALLOW_COPY_AND_ASSIGN(ObCheckTransferTabletBackfillArg);
};

struct ObCheckTransferTabletBackfillRes final
{
  OB_UNIS_VERSION(1);
public:
  ObCheckTransferTabletBackfillRes();
  ~ObCheckTransferTabletBackfillRes() {}
  void reset();
  TO_STRING_KV(K_(backfill_finished));
  bool backfill_finished_;
};

struct ObStorageReplaceMemberArg final
{
  OB_UNIS_VERSION(1);
public:
  ObStorageReplaceMemberArg();
  ~ObStorageReplaceMemberArg() {}
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(added_member), K_(removed_member), K_(ls_transfer_scn));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  common::ObMember added_member_;
  common::ObMember removed_member_;
  share::SCN ls_transfer_scn_;
private:
  DISALLOW_COPY_AND_ASSIGN(ObStorageReplaceMemberArg);
};

struct ObStorageAddMemberArg final
{
  OB_UNIS_VERSION(1);
public:
  ObStorageAddMemberArg();
  ~ObStorageAddMemberArg() {}
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(member), K_(new_replica_num), K_(ls_transfer_scn));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  common::ObMember member_;
  int64_t new_replica_num_;
  share::SCN ls_transfer_scn_;
private:
  DISALLOW_COPY_AND_ASSIGN(ObStorageAddMemberArg);
};

struct ObStorageSwitchLToFArg final
{
  OB_UNIS_VERSION(1);
public:
  ObStorageSwitchLToFArg();
  ~ObStorageSwitchLToFArg() {}
  bool is_valid() const;
  void reset();
  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(learner), K_(new_replica_num), K_(ls_transfer_scn));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  common::ObMember learner_;
  int64_t new_replica_num_;
  share::SCN ls_transfer_scn_;
private:
  DISALLOW_COPY_AND_ASSIGN(ObStorageSwitchLToFArg);
};

struct ObStorageChangeMemberRes final
{
  OB_UNIS_VERSION(1);
public:
  ObStorageChangeMemberRes();
  ~ObStorageChangeMemberRes() {}
  void reset();
  TO_STRING_KV(K_(change_succ));
  bool change_succ_;
private:
  DISALLOW_COPY_AND_ASSIGN(ObStorageChangeMemberRes);
};

// Fetch ls meta and all tablet metas by stream reader.
struct ObCopyLSViewArg final
{
  OB_UNIS_VERSION(1);
public:
  ObCopyLSViewArg();
  ~ObCopyLSViewArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
};

struct ObCheckStartTransferTabletsRes final
{
  OB_UNIS_VERSION(1);
public:
  ObCheckStartTransferTabletsRes();
  ~ObCheckStartTransferTabletsRes() {}
  void reset();
  TO_STRING_KV(K_(change_succ));
  bool change_succ_;
private:
  DISALLOW_COPY_AND_ASSIGN(ObCheckStartTransferTabletsRes);
};

struct ObStorageBlockTxArg final
{
  OB_UNIS_VERSION(1);
public:
  ObStorageBlockTxArg();
  ~ObStorageBlockTxArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(gts));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  share::SCN gts_;
};

struct ObStorageKillTxArg final
{
  OB_UNIS_VERSION(1);
public:
  ObStorageKillTxArg();
  ~ObStorageKillTxArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(gts));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  share::SCN gts_;
};

struct ObStorageUnBlockTxArg final
{
  OB_UNIS_VERSION(1);
public:
  ObStorageUnBlockTxArg();
  ~ObStorageUnBlockTxArg() {}
  bool is_valid() const;
  void reset();

  TO_STRING_KV(K_(tenant_id), K_(ls_id), K_(gts));
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  share::SCN gts_;
};

//src
class ObStorageRpcProxy : public obrpc::ObRpcProxy
{
public:
  static const int64_t STREAM_RPC_TIMEOUT = 30 * 1000 * 1000LL; // 30s
  DEFINE_TO(ObStorageRpcProxy);
  //stream
  RPC_SS(PR5 fetch_macro_block, OB_HA_FETCH_MACRO_BLOCK, (ObCopyMacroBlockRangeArg), common::ObDataBuffer);
  RPC_SS(PR5 fetch_tablet_info, OB_HA_FETCH_TABLET_INFO, (ObCopyTabletInfoArg), common::ObDataBuffer);
  RPC_SS(PR5 fetch_tablet_sstable_info, OB_HA_FETCH_SSTABLE_INFO, (ObCopyTabletsSSTableInfoArg), common::ObDataBuffer);
  RPC_SS(PR5 fetch_sstable_macro_info, OB_HA_FETCH_SSTABLE_MACRO_INFO, (ObCopySSTableMacroRangeInfoArg), common::ObDataBuffer);
  RPC_SS(PR5 lob_query, OB_LOB_QUERY, (ObLobQueryArg), common::ObDataBuffer);
  RPC_SS(PR5 fetch_transfer_tablet_info, OB_FETCH_TRANSFER_TABLET_INFO, (ObTransferTabletInfoArg), common::ObDataBuffer);
  RPC_SS(PR5 fetch_ls_view, OB_HA_FETCH_LS_VIEW, (ObCopyLSViewArg), common::ObDataBuffer);

  RPC_S(PR5 fetch_ls_member_list, OB_HA_FETCH_LS_MEMBER_LIST, (ObFetchLSMemberListArg), ObFetchLSMemberListInfo);
  RPC_S(PR5 fetch_ls_meta_info, OB_HA_FETCH_LS_META_INFO, (ObFetchLSMetaInfoArg), ObFetchLSMetaInfoResp);
  RPC_S(PR5 fetch_ls_info, OB_HA_FETCH_LS_INFO, (ObCopyLSInfoArg), ObCopyLSInfo);
  RPC_S(PR5 notify_restore_tablets, OB_HA_NOTIFY_RESTORE_TABLETS, (ObNotifyRestoreTabletsArg), ObNotifyRestoreTabletsResp);
  RPC_S(PR5 inquire_restore, OB_HA_NOTIFY_FOLLOWER_RESTORE, (ObInquireRestoreArg), ObInquireRestoreResp);
  RPC_S(PR5 update_ls_meta, OB_HA_UPDATE_LS_META, (ObRestoreUpdateLSMetaArg));
  RPC_S(PR5 check_start_transfer_tablets, OB_CHECK_START_TRANSFER_TABLETS, (ObTransferTabletInfoArg));
  RPC_S(PR5 get_ls_active_trans_count, OB_GET_LS_ACTIVE_TRANSACTION_COUNT, (ObGetLSActiveTransCountArg), ObGetLSActiveTransCountRes);
  RPC_S(PR5 get_transfer_start_scn, OB_GET_TRANSFER_START_SCN, (ObGetTransferStartScnArg), ObGetTransferStartScnRes);
  RPC_S(PR5 fetch_ls_replay_scn, OB_HA_FETCH_LS_REPLAY_SCN, (ObFetchLSReplayScnArg), ObFetchLSReplayScnRes);
  RPC_S(PR5 check_transfer_tablet_backfill_completed, OB_HA_CHECK_TRANSFER_TABLET_BACKFILL, (ObCheckTransferTabletBackfillArg), ObCheckTransferTabletBackfillRes);
  RPC_S(PR5 replace_member, OB_HA_REPLACE_MEMBER, (ObStorageReplaceMemberArg), ObStorageChangeMemberRes);
  RPC_S(PR5 add_member, OB_HA_ADD_MEMBER, (ObStorageAddMemberArg), ObStorageChangeMemberRes);
  RPC_S(PR5 switch_learner_to_acceptor, OB_HA_SWITCH_LEARNER_TO_ACCEPTOR, (ObStorageSwitchLToFArg), ObStorageChangeMemberRes);
  RPC_S(PR5 block_tx, OB_HA_BLOCK_TX, (ObStorageBlockTxArg));
  RPC_S(PR5 kill_tx, OB_HA_KILL_TX, (ObStorageKillTxArg));
  RPC_S(PR5 unblock_tx, OB_HA_UNBLOCK_TX, (ObStorageUnBlockTxArg));
};

template <ObRpcPacketCode RPC_CODE>
class ObStorageStreamRpcP : public ObRpcProcessor<obrpc::ObStorageRpcProxy::ObRpc<RPC_CODE> >
{
public:
  explicit ObStorageStreamRpcP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObStorageStreamRpcP() {}
protected:
  template <typename Data>
  int fill_data(const Data &data);
  template <typename Data>
  int fill_data_list(ObIArray<Data> &data_list);
  template<typename Data>
  int fill_data_immediate(const Data &data);
  int fill_buffer(blocksstable::ObBufferReader &data);
  int flush_and_wait();
  int alloc_buffer();

  int is_follower_ls(logservice::ObLogService *log_srv, ObLS *ls, bool &is_ls_follower);
protected:
  common::ObInOutBandwidthThrottle *bandwidth_throttle_;
  int64_t last_send_time_;
  common::ObArenaAllocator allocator_;
  static const int64_t FLUSH_TIME_INTERVAL = ObStorageRpcProxy::STREAM_RPC_TIMEOUT / 2;
};

class ObHAFetchMacroBlockP: public ObStorageStreamRpcP<OB_HA_FETCH_MACRO_BLOCK>
{
public:
  explicit ObHAFetchMacroBlockP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObHAFetchMacroBlockP() {}
protected:
  int process();
private:
  int64_t total_macro_block_count_;
};

class ObFetchTabletInfoP :
    public ObStorageStreamRpcP<OB_HA_FETCH_TABLET_INFO>
{
public:
  explicit ObFetchTabletInfoP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObFetchTabletInfoP() {}
protected:
  int process();
};

class ObFetchSSTableInfoP :
    public ObStorageStreamRpcP<OB_HA_FETCH_SSTABLE_INFO>
{
public:
  explicit ObFetchSSTableInfoP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObFetchSSTableInfoP() {}
protected:
  int process();
private:
  int build_tablet_sstable_info_(ObLS *ls);
  int build_sstable_info_(
      const obrpc::ObCopyTabletSSTableInfoArg &arg,
      ObLS *ls);
};

class ObFetchLSInfoP :
    public ObStorageRpcProxy::Processor<OB_HA_FETCH_LS_INFO>
{
public:
  explicit ObFetchLSInfoP();
  virtual ~ObFetchLSInfoP() { }
protected:
  int process();
};

class ObFetchLSMetaInfoP :
    public ObStorageRpcProxy::Processor<OB_HA_FETCH_LS_META_INFO>
{
public:
  explicit ObFetchLSMetaInfoP();
  virtual ~ObFetchLSMetaInfoP() { }
protected:
  int process();
private:
  int check_has_transfer_logical_table_(storage::ObLS *ls);
};

class ObFetchLSMemberListP:
    public ObStorageRpcProxy::Processor<OB_HA_FETCH_LS_MEMBER_LIST>
{
public:
  explicit ObFetchLSMemberListP();
  virtual ~ObFetchLSMemberListP() { }
protected:
  int process();
};

class ObFetchSSTableMacroInfoP :
    public ObStorageStreamRpcP<OB_HA_FETCH_SSTABLE_MACRO_INFO>
{
public:
  explicit ObFetchSSTableMacroInfoP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObFetchSSTableMacroInfoP() {}
protected:
  int process();
private:
  int fetch_sstable_macro_info_header_();
  int fetch_sstable_macro_range_info_(
      const obrpc::ObCopySSTableMacroRangeInfoHeader &header);
};

class ObNotifyRestoreTabletsP :
    public ObStorageStreamRpcP<OB_HA_NOTIFY_RESTORE_TABLETS>
{
public:
  explicit ObNotifyRestoreTabletsP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObNotifyRestoreTabletsP() {}
protected:
  int process();
};

class ObInquireRestoreP :
    public ObStorageStreamRpcP<OB_HA_NOTIFY_FOLLOWER_RESTORE>
{
public:
  explicit ObInquireRestoreP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObInquireRestoreP() {}
protected:
  int process();
};

class ObUpdateLSMetaP :
    public ObStorageStreamRpcP<OB_HA_UPDATE_LS_META>
{
public:
  explicit ObUpdateLSMetaP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObUpdateLSMetaP() {}
protected:
  int process();
};

class ObCheckStartTransferTabletsP : public ObStorageStreamRpcP<OB_CHECK_START_TRANSFER_TABLETS>
{
public:
  explicit ObCheckStartTransferTabletsP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObCheckStartTransferTabletsP() {}
protected:
  int process();
private:
  int check_start_transfer_out_tablets_();
  int check_start_transfer_in_tablets_();
  // Major sstable or ddl sstable needs to exist in src_tablet
  int check_transfer_out_tablet_sstable_(const ObTablet *tablet);
};

class ObGetLSActiveTransCountP : public ObStorageStreamRpcP<OB_GET_LS_ACTIVE_TRANSACTION_COUNT>
{
public:
  explicit ObGetLSActiveTransCountP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObGetLSActiveTransCountP() {}
protected:
  int process();
};

class ObGetTransferStartScnP : public ObStorageStreamRpcP<OB_GET_TRANSFER_START_SCN>
{
public:
  explicit ObGetTransferStartScnP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObGetTransferStartScnP() {}
protected:
  int process();
};

class ObFetchTransferTabletsInfoP :
    public ObStorageStreamRpcP<OB_FETCH_TRANSFER_TABLET_INFO>
{
public:
  explicit ObFetchTransferTabletsInfoP(
      common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObFetchTransferTabletsInfoP() {}
protected:
  int process();
};

class ObFetchLSReplayScnP:
    public ObStorageStreamRpcP<OB_HA_FETCH_LS_REPLAY_SCN>
{
public:
  explicit ObFetchLSReplayScnP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObFetchLSReplayScnP() {}
protected:
  int process();
};
class ObCheckTransferTabletsBackfillP:
    public ObStorageStreamRpcP<OB_HA_CHECK_TRANSFER_TABLET_BACKFILL>
{
public:
  explicit ObCheckTransferTabletsBackfillP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObCheckTransferTabletsBackfillP() {}
protected:
  int process();
private:
  int check_has_transfer_table_(const share::ObTransferTabletInfo &tablet_info,
      storage::ObLS *ls, bool &has_transfer_table);
};

class ObStorageReplaceMemberP:
    public ObStorageStreamRpcP<OB_HA_REPLACE_MEMBER>
{
public:
  explicit ObStorageReplaceMemberP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObStorageReplaceMemberP() {}
protected:
  int process();
};

class ObStorageAddMemberP:
    public ObStorageStreamRpcP<OB_HA_ADD_MEMBER>
{
public:
  explicit ObStorageAddMemberP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObStorageAddMemberP() {}
protected:
  int process();
};

class ObStorageSwitchLearnerToAcceptorP:
    public ObStorageStreamRpcP<OB_HA_SWITCH_LEARNER_TO_ACCEPTOR>
{
public:
  explicit ObStorageSwitchLearnerToAcceptorP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObStorageSwitchLearnerToAcceptorP() {}
protected:
  int process();
};

class ObLobQueryP : public ObStorageStreamRpcP<OB_LOB_QUERY>
{
public:
  explicit ObLobQueryP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObLobQueryP() {}
protected:
  int process();
private:
  int process_read();
  int process_getlength();
};


// Stream get ls meta and all tablet meta
class ObStorageFetchLSViewP:
    public ObStorageStreamRpcP<OB_HA_FETCH_LS_VIEW>
{
public:
  explicit ObStorageFetchLSViewP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObStorageFetchLSViewP() {}

protected:
  int process();
};

class ObStorageBlockTxP:
    public ObStorageStreamRpcP<OB_HA_BLOCK_TX>
{
public:
  explicit ObStorageBlockTxP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObStorageBlockTxP() {}
protected:
  int process();
private:
  int check_before_kill_(storage::ObLS *ls, const share::ObLSID &ls_id, const int64_t timeout);
};

class ObStorageKillTxP:
    public ObStorageStreamRpcP<OB_HA_KILL_TX>
{
public:
  explicit ObStorageKillTxP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObStorageKillTxP() {}
protected:
  int process();
};

class ObStorageUnBlockTxP:
    public ObStorageStreamRpcP<OB_HA_UNBLOCK_TX>
{
public:
  explicit ObStorageUnBlockTxP(common::ObInOutBandwidthThrottle *bandwidth_throttle);
  virtual ~ObStorageUnBlockTxP() {}
protected:
  int process();
};

} // obrpc


namespace storage
{
//dst
class ObIStorageRpc
{
public:
  ObIStorageRpc() {}
  virtual ~ObIStorageRpc() {}
  virtual int init(
      obrpc::ObStorageRpcProxy *rpc_proxy,
      const common::ObAddr &self,
      obrpc::ObCommonRpcProxy *rs_rpc_proxy) = 0;
  virtual void destroy() = 0;
public:
  virtual int post_ls_info_request(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      obrpc::ObCopyLSInfo &ls_info) = 0;
  virtual int post_ls_meta_info_request(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      obrpc::ObFetchLSMetaInfoResp &ls_info) = 0;
  virtual int post_ls_member_list_request(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      obrpc::ObFetchLSMemberListInfo &ls_info) = 0;
  virtual int post_ls_disaster_recovery_res(const common::ObAddr &server,
                           const obrpc::ObDRTaskReplyResult &res) = 0;

  // Notify follower restore some tablets from leader.
  virtual int notify_restore_tablets(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &follower_info,
      const share::ObLSID &ls_id,
      const int64_t &proposal_id,
      const common::ObIArray<common::ObTabletID>& tablet_id_array,
      const share::ObLSRestoreStatus &restore_status,
      obrpc::ObNotifyRestoreTabletsResp &restore_resp) = 0;

  // inquire restore status from src.
  virtual int inquire_restore(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const share::ObLSRestoreStatus &restore_status,
      obrpc::ObInquireRestoreResp &restore_resp) = 0;

  virtual int update_ls_meta(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &dest_info,
      const storage::ObLSMetaPackage &ls_meta) = 0;

  virtual int check_start_transfer_tablets(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &src_ls_id,
      const share::ObLSID &dest_ls_id,
      const common::ObIArray<share::ObTransferTabletInfo>& tablet_array) = 0;

  virtual int get_ls_active_trans_count(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      int64_t &active_trans_count) = 0;

  virtual int get_transfer_start_scn(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const common::ObIArray<share::ObTransferTabletInfo> &tablet_list,
      share::SCN &transfer_start_scn) = 0;

  virtual int fetch_ls_replay_scn(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      share::SCN &ls_replay_scn) = 0;
  virtual int check_tablets_logical_table_replaced(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &dest_ls_id,
      const common::ObIArray<share::ObTransferTabletInfo> &tablet_array,
      bool &replace_finished) = 0;

  virtual int replace_member(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const common::ObMember &added_member,
      const common::ObMember &removed_member,
      const share::SCN &ls_transfer_scn,
      const int64_t timeout) = 0;
  virtual int add_member(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const common::ObMember &added_member,
      const int64_t new_replica_num,
      const share::SCN &ls_transfer_scn,
      const int64_t timeout) = 0;
  virtual int switch_learner_to_acceptor(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const common::ObMember &learner,
      const int64_t paxos_replica_num,
      const share::SCN &ls_transfer_scn,
      const int64_t timeout) = 0;
  virtual int block_tx(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const share::SCN &gts) = 0;
  virtual int kill_tx(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const share::SCN &gts) = 0;
  virtual int unblock_tx(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const share::SCN &gts) = 0;
};

class ObStorageRpc: public ObIStorageRpc
{
public:
  ObStorageRpc();
  ~ObStorageRpc();
  int init(obrpc::ObStorageRpcProxy *rpc_proxy,
      const common::ObAddr &self, obrpc::ObCommonRpcProxy *rs_rpc_proxy);
  void destroy();
public:
  virtual int post_ls_info_request(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      obrpc::ObCopyLSInfo &ls_meta);
  virtual int post_ls_meta_info_request(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      obrpc::ObFetchLSMetaInfoResp &ls_info);
  virtual int post_ls_member_list_request(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      obrpc::ObFetchLSMemberListInfo &ls_info);
  virtual int post_ls_disaster_recovery_res(const common::ObAddr &server,
                           const obrpc::ObDRTaskReplyResult &res);

  // Notify follower restore some tablets from leader.
  virtual int notify_restore_tablets(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &follower_info,
      const share::ObLSID &ls_id,
      const int64_t &proposal_id,
      const common::ObIArray<common::ObTabletID>& tablet_id_array,
      const share::ObLSRestoreStatus &restore_status,
      obrpc::ObNotifyRestoreTabletsResp &restore_resp);

  // inquire restore status from src.
  virtual int inquire_restore(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const share::ObLSRestoreStatus &restore_status,
      obrpc::ObInquireRestoreResp &restore_resp);

  virtual int update_ls_meta(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &dest_info,
      const storage::ObLSMetaPackage &ls_meta);

  virtual int check_start_transfer_tablets(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &src_ls_id,
      const share::ObLSID &dest_ls_id,
      const common::ObIArray<share::ObTransferTabletInfo>& tablet_array);

  virtual int get_ls_active_trans_count(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      int64_t &active_trans_count);

  virtual int get_transfer_start_scn(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const common::ObIArray<share::ObTransferTabletInfo> &tablet_list,
      share::SCN &transfer_start_scn);

  virtual int fetch_ls_replay_scn(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      share::SCN &ls_replay_scn);
  virtual int check_tablets_logical_table_replaced(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &dest_ls_id,
      const common::ObIArray<share::ObTransferTabletInfo> &tablet_array,
      bool &replace_finished);
  virtual int replace_member(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const common::ObMember &added_member,
      const common::ObMember &removed_member,
      const share::SCN &ls_transfer_scn,
      const int64_t timeout);
  virtual int add_member(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const common::ObMember &added_member,
      const int64_t new_replica_num,
      const share::SCN &ls_transfer_scn,
      const int64_t timeout);
  virtual int switch_learner_to_acceptor(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const common::ObMember &learner,
      const int64_t new_replica_num,
      const share::SCN &ls_transfer_scn,
      const int64_t timeout);
  virtual int block_tx(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const share::SCN &gts);
  virtual int kill_tx(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const share::SCN &gts);
  virtual int unblock_tx(
      const uint64_t tenant_id,
      const ObStorageHASrcInfo &src_info,
      const share::ObLSID &ls_id,
      const share::SCN &gts);

private:
  bool is_inited_;
  obrpc::ObStorageRpcProxy *rpc_proxy_;
  common::ObAddr self_;
  obrpc::ObCommonRpcProxy *rs_rpc_proxy_;
};

template<obrpc::ObRpcPacketCode RPC_CODE>
class ObStorageStreamRpcReader
{
public:
  ObStorageStreamRpcReader();
  virtual ~ObStorageStreamRpcReader() {}
  int init(common::ObInOutBandwidthThrottle &bandwidth_throttle);
  int fetch_next_buffer_if_need();
  int check_need_fetch_next_buffer(bool &need_fectch);
  int fetch_next_buffer();
  template<typename Data>
  int fetch_and_decode(Data &data);
  template<typename Data>
  int fetch_and_decode(common::ObIAllocator &allocator, Data &data);
  template<typename Data>
  int fetch_and_decode_list(common::ObIAllocator &allocator,
                            common::ObIArray<Data> &data_list);
  template<typename Data>
  int fetch_and_decode_list(
      const int64_t data_list_count,
      common::ObIArray<Data> &data_list);
  common::ObDataBuffer &get_rpc_buffer() { return rpc_buffer_; }
  const common::ObAddr &get_dst_addr() const { return handle_.get_dst_addr(); }
  obrpc::ObStorageRpcProxy::SSHandle<RPC_CODE> &get_handle() { return handle_; }
  void reuse()
  {
    rpc_buffer_.get_position() = 0;
    rpc_buffer_parse_pos_ = 0;
  }
private:
  bool is_inited_;
  obrpc::ObStorageRpcProxy::SSHandle<RPC_CODE> handle_;
  common::ObInOutBandwidthThrottle *bandwidth_throttle_;
  common::ObDataBuffer rpc_buffer_;
  int64_t rpc_buffer_parse_pos_;
  common::ObArenaAllocator allocator_;
  int64_t last_send_time_;
  int64_t data_size_;
};

} // storage
} // oceanbase

#include "storage/ob_storage_rpc.ipp"

#endif //OCEANBASE_STORAGE_OB_PARTITION_SERVICE_RPC_
