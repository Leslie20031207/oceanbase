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

#ifndef OCEABASE_STORAGE_HA_SRC_PROVIDER_
#define OCEABASE_STORAGE_HA_SRC_PROVIDER_

#include "share/ob_srv_rpc_proxy.h"  // ObPartitionServiceRpcProxy
#include "storage/ob_storage_rpc.h"
#include "ob_storage_ha_struct.h"
#include "common/ob_learner_list.h"
#include "ob_storage_ha_utils.h"

namespace oceanbase {
namespace storage {

class ObStorageHAGetMemberHelper
{
public:
  ObStorageHAGetMemberHelper();
  virtual ~ObStorageHAGetMemberHelper();
  int init(storage::ObStorageRpc *storage_rpc);
  int get_ls_member_list(const uint64_t tenant_id, const share::ObLSID &ls_id,
      common::ObIArray<common::ObAddr> &addr_list);
  int get_ls_member_list_and_learner_list(
      const uint64_t tenant_id, const share::ObLSID &ls_id, const bool need_learner_list,
      common::ObAddr &leader_addr, common::GlobalLearnerList &learner_list,
      common::ObIArray<common::ObAddr> &member_list);
  virtual int get_ls_leader(const uint64_t tenant_id, const share::ObLSID &ls_id, common::ObAddr &addr);
  virtual int get_ls(const share::ObLSID &ls_id, ObLSHandle &ls_handle);
  virtual bool check_tenant_primary();
  // According to the replica type, determine whether to get learner list.
  // F replica get member list, R replica get member list and learner list.
  int get_member_list_by_replica_type(
      const uint64_t tenant_id, const share::ObLSID &ls_id, const common::ObReplicaMember &dst,
      ObLSMemberListInfo &info);

private:
  int fetch_ls_member_list_and_learner_list_(const uint64_t tenant_id, const share::ObLSID &ls_id, const bool need_learner_list,
      common::ObAddr &addr, common::GlobalLearnerList &learner_list, common::ObIArray<common::ObAddr> &member_list);
  virtual int get_ls_member_list_and_learner_list_(const uint64_t tenant_id, const share::ObLSID &ls_id,
      const bool need_learner_list, common::ObAddr &leader_addr,
      common::GlobalLearnerList &learner_list, common::ObIArray<common::ObAddr> &member_list);
  int filter_dest_replica_(
      const common::ObReplicaMember &dst,
      common::GlobalLearnerList &learner_list);

private:
  bool is_inited_;
  storage::ObStorageRpc *storage_rpc_;
  DISALLOW_COPY_AND_ASSIGN(ObStorageHAGetMemberHelper);
};

class ObStorageHASrcProvider {
public:
  enum ChooseSourcePolicy : uint8_t
  {
    IDC = 0,
    REGION = 1,
    CHECKPOINT = 2,
    RECOMMEND = 3,
    C_TYPE_REPLICA = 4,
    MAX_POLICY
  };

  ObStorageHASrcProvider();
  virtual ~ObStorageHASrcProvider();
  int init(const ObMigrationChooseSrcHelperInitParam &param,
      const ChooseSourcePolicy policy_type,
      storage::ObStorageRpc *storage_rpc,
      ObStorageHAGetMemberHelper *member_helper);
  virtual int choose_ob_src(
      const ObMigrationOpArg &arg, common::ObAddr &chosen_src_addr) = 0;

  inline uint64_t get_tenant_id() const { return tenant_id_; }
  const share::ObLSID &get_ls_id() const { return ls_id_; }
  ObMigrationOpType::TYPE get_type() const { return type_; }
  storage::ObStorageRpc *get_storage_rpc() const { return storage_rpc_; }
  const share::SCN &get_local_clog_checkpoint_scn() const { return local_clog_checkpoint_scn_; }
  const share::SCN &get_palf_parent_checkpoint_scn() const { return palf_parent_checkpoint_scn_; }
  ChooseSourcePolicy get_policy_type() const { return policy_type_; }

  const static char *ObChooseSourcePolicyStr[static_cast<int64_t>(ChooseSourcePolicy::MAX_POLICY)];
  const static char *get_policy_str(const ChooseSourcePolicy policy_type);
  int check_tenant_primary(bool &is_primary);

protected:
  // The validity assessment of replicas includes:
  // server_version: dest server_version >= src server_version
  // restore_status: if restore_status of ls is fail, migration needs to wait.
  // migration_status: OB_MIGRATION_STATUS_NONE
  // replica type: F replica could serve as the source of F replica and R replica,
  // while R replica could only serve as the source of R replica
  // source checkpoint scn must be greater than or equal than palf_parent_checkpoint_scn_ and local_clog_checkpoint_scn_
  int check_replica_validity(
      const common::ObAddr &addr, const common::ObReplicaMember &dst,
      const common::GlobalLearnerList &learner_list, obrpc::ObFetchLSMetaInfoResp &ls_info);
protected:
  bool is_inited_;
  ObLSMemberListInfo member_list_info_;

private:
  int fetch_ls_meta_info_(const uint64_t tenant_id, const share::ObLSID &ls_id, const common::ObAddr &member_addr,
      obrpc::ObFetchLSMetaInfoResp &ls_meta_info);
  int check_replica_type_(
      const common::ObAddr &addr,
      const common::ObReplicaMember &dst,
      const common::GlobalLearnerList &learner_list,
      bool &is_replica_type_valid);
  int init_palf_parent_checkpoint_scn_(const uint64_t tenant_id, const share::ObLSID &ls_id,
      const share::SCN &local_clog_checkpoint_scn, const common::ObReplicaType replica_type);

  int get_palf_parent_checkpoint_scn_from_rpc_(const uint64_t tenant_id, const share::ObLSID &ls_id,
      const common::ObReplicaType replica_type, share::SCN &parent_checkpoint_scn);
  int get_palf_parent_addr_(const uint64_t tenant_id, const share::ObLSID &ls_id,
      const common::ObReplicaType replica_type, common::ObAddr &parent_addr);
  int check_replica_type_for_normal_replica_(
      const common::ObAddr &addr,
      const common::ObReplicaMember &dst,
      const common::GlobalLearnerList &learner_list,
      bool &is_replica_type_valid);
  int check_replica_type_for_c_replica_(
      const common::ObAddr &addr,
      const common::ObReplicaMember &dst,
      const common::GlobalLearnerList &learner_list,
      bool &is_replica_type_valid);

private:
  uint64_t tenant_id_;
  share::ObLSID ls_id_;
  ObMigrationOpType::TYPE type_;
  share::SCN local_clog_checkpoint_scn_;
  share::SCN palf_parent_checkpoint_scn_;
  ObStorageHAGetMemberHelper *member_helper_;
  storage::ObStorageRpc *storage_rpc_;
  ChooseSourcePolicy policy_type_;
  DISALLOW_COPY_AND_ASSIGN(ObStorageHASrcProvider);
};

class ObMigrationSrcByLocationProvider : public ObStorageHASrcProvider
{
public:
  ObMigrationSrcByLocationProvider();
  virtual ~ObMigrationSrcByLocationProvider();
  int init(const ObMigrationChooseSrcHelperInitParam &param,
      const ChooseSourcePolicy policy_type,
      storage::ObStorageRpc *storage_rpc,
      ObStorageHAGetMemberHelper *member_helper);
  virtual int choose_ob_src(
      const ObMigrationOpArg &arg, common::ObAddr &chosen_src_addr) override;

protected:
  int inner_choose_ob_src(
      const common::ObAddr &leader_addr, const common::GlobalLearnerList &learner_list,
      const common::ObIArray<common::ObAddr> &addr_list, const ObMigrationOpArg &arg,
      common::ObAddr &choosen_src_addr);
  int divide_addr_list(
      const common::ObIArray<common::ObAddr> &addr_list,
      const common::ObReplicaMember &dst,
      common::ObIArray<common::ObAddr> &sorted_addr_list,
      int64_t &idc_end_index,
      int64_t &region_end_index);
  int find_src(
      const common::ObIArray<common::ObAddr> &sorted_addr_list,
      const int64_t start_index,
      const int64_t end_index,
      const common::GlobalLearnerList &learner_list,
      const common::ObAddr &leader_addr,
      const common::ObReplicaMember &dst,
      common::ObAddr &choosen_src_addr);
private:
  void set_locality_manager_(ObLocalityManager *locality_manager);
  int get_server_region_and_idc_(
      const common::ObAddr &addr, common::ObRegion &region, common::ObIDC &idc);
private:
  ObLocalityManager *locality_manager_;

  DISALLOW_COPY_AND_ASSIGN(ObMigrationSrcByLocationProvider);
};

class ObMigrationSrcByCheckpointProvider : public ObStorageHASrcProvider
{
public:
  ObMigrationSrcByCheckpointProvider();
  virtual ~ObMigrationSrcByCheckpointProvider();
  int init(const ObMigrationChooseSrcHelperInitParam &param,
      const ChooseSourcePolicy policy_type,
      storage::ObStorageRpc *storage_rpc,
      ObStorageHAGetMemberHelper *member_helper);
  virtual int choose_ob_src(
      const ObMigrationOpArg &arg, common::ObAddr &chosen_src_addr) override;

private:
  int inner_choose_ob_src_(
      const common::ObAddr &leader_addr, const common::GlobalLearnerList &learner_list,
      const common::ObIArray<common::ObAddr> &addr_list, const ObMigrationOpArg &arg,
      common::ObAddr &choosen_src_addr);

  DISALLOW_COPY_AND_ASSIGN(ObMigrationSrcByCheckpointProvider);
};

class ObRSRecommendSrcProvider : public ObStorageHASrcProvider
{
public:
  ObRSRecommendSrcProvider();
  virtual ~ObRSRecommendSrcProvider();
  int init(const ObMigrationChooseSrcHelperInitParam &param,
      const ChooseSourcePolicy policy_type,
      storage::ObStorageRpc *storage_rpc,
      ObStorageHAGetMemberHelper *member_helper);
  int choose_ob_src(
      const ObMigrationOpArg &arg, common::ObAddr &chosen_src_addr);
private:
  int check_replica_validity_(const int64_t cluster_id, const common::ObIArray<common::ObAddr> &addr_list,
      const common::ObAddr &addr, const common::ObReplicaMember &dst,
      const common::GlobalLearnerList &learner_list);
  DISALLOW_COPY_AND_ASSIGN(ObRSRecommendSrcProvider);
};

class ObCTypeReplicaSrcProvider : public ObStorageHASrcProvider
{
public:
  ObCTypeReplicaSrcProvider();
  virtual ~ObCTypeReplicaSrcProvider();
  int init(const ObMigrationChooseSrcHelperInitParam &param,
      const ChooseSourcePolicy policy_type,
      storage::ObStorageRpc *storage_rpc,
      ObStorageHAGetMemberHelper *member_helper);
  int choose_ob_src(
      const ObMigrationOpArg &arg, common::ObAddr &chosen_src_addr);

private:
  int inner_choose_ob_src_(
      const common::ObIArray<common::ObAddr> &addr_list,
      const common::GlobalLearnerList &learner_list,
      const ObMigrationOpArg &arg, common::ObAddr &choosen_src_addr);
  int get_c_replica_(
      const common::ObAddr &addr,
      const common::GlobalLearnerList &learner_list,
      ObIArray<common::ObAddr> &c_replica_list) const;
  DISALLOW_COPY_AND_ASSIGN(ObCTypeReplicaSrcProvider);
};

class ObStorageHAChooseSrcHelper final
{
public:
  ObStorageHAChooseSrcHelper();
  ~ObStorageHAChooseSrcHelper();
  int init(const ObMigrationChooseSrcHelperInitParam &param, const ObStorageHASrcProvider::ChooseSourcePolicy policy,
      storage::ObStorageRpc *storage_rpc, ObStorageHAGetMemberHelper *member_helper);
  int get_available_src(const ObMigrationOpArg &arg, ObStorageHASrcInfo &src_info);
  static int get_policy_type(const ObMigrationOpArg &arg, const uint64_t tenant_id,
      bool enable_choose_source_policy, const char *policy_str,
      const common::GlobalLearnerList &learner_list,
      ObStorageHASrcProvider::ChooseSourcePolicy &policy);

private:
  int init_rs_recommend_source_provider_(const ObMigrationChooseSrcHelperInitParam &param,
      storage::ObStorageRpc *storage_rpc,
      ObStorageHAGetMemberHelper *member_helper);
  int init_choose_source_by_location_provider_(
      const ObMigrationChooseSrcHelperInitParam &param,
      const ObStorageHASrcProvider::ChooseSourcePolicy policy, storage::ObStorageRpc *storage_rpc,
      ObStorageHAGetMemberHelper *member_helper);
  int init_choose_source_by_checkpoint_provider_(
      const ObMigrationChooseSrcHelperInitParam &param,
      storage::ObStorageRpc *storage_rpc,
      ObStorageHAGetMemberHelper *member_helper);
  int init_c_type_replica_source_provider_(
      const ObMigrationChooseSrcHelperInitParam &param,
      storage::ObStorageRpc *storage_rpc, ObStorageHAGetMemberHelper *member_helper);
  void errsim_test_(const ObMigrationOpArg &arg, ObStorageHASrcInfo &src_info);
  ObStorageHASrcProvider * get_provider() const { return provider_; }
  static int check_c_replica_migration_policy_(const uint64_t tenant_id, const share::ObLSID &ls_id,
      const common::ObReplicaMember &dst, const common::GlobalLearnerList &learner_list, bool &use_c_replica_policy);
  static int check_exist_c_replica_(const uint64_t tenant_id,
      const share::ObLSID &ls_id, const common::GlobalLearnerList &learner_list, bool &exist_c_replica);
private:
  ObStorageHASrcProvider *provider_;
  storage::ObStorageRpc *storage_rpc_;
  common::ObArenaAllocator allocator_;
  bool is_inited_;
  DISALLOW_COPY_AND_ASSIGN(ObStorageHAChooseSrcHelper);
};

}  // namespace storage
}  // namespace oceanbase
#endif
