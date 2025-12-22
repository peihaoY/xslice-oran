#include "../../../../src/xApp/e42_xapp_api.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_struct.h"
#include "../../../../src/sm/rc_sm/ie/ir/ran_param_list.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../../../../src/sm/rc_sm/rc_sm_id.h"
//KPM  MAC
#include "../../../../src/util/alg_ds/alg/defer.h"
#include "../../../../src/util/time_now_us.h"
#include "../../../../src/util/alg_ds/ds/lock_guard/lock_guard.h"
#include "../../../../src/util/e.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
//KPM  MAC
#include <signal.h>
#include <pthread.h>

//******************************************SLICE Begin*************************************//
typedef enum{
    DRX_parameter_configuration_7_6_3_1 = 1,
    SR_periodicity_configuration_7_6_3_1 = 2,
    SPS_parameters_configuration_7_6_3_1 = 3,
    Configured_grant_control_7_6_3_1 = 4,
    CQI_table_configuration_7_6_3_1 = 5,
    Slice_level_PRB_quotal_7_6_3_1 = 6,
} rc_ctrl_service_style_2_act_id_e;

static
e2sm_rc_ctrl_hdr_frmt_1_t gen_rc_ctrl_hdr_frmt_1(ue_id_e2sm_t ue_id, uint32_t ric_style_type, uint16_t ctrl_act_id)
{
  e2sm_rc_ctrl_hdr_frmt_1_t dst = {0};

  // 6.2.2.6
  dst.ue_id = cp_ue_id_e2sm(&ue_id);

  dst.ric_style_type = ric_style_type;
  dst.ctrl_act_id = ctrl_act_id;

  return dst;
}

static
e2sm_rc_ctrl_hdr_t gen_rc_ctrl_hdr(e2sm_rc_ctrl_hdr_e hdr_frmt, ue_id_e2sm_t ue_id, uint32_t ric_style_type, uint16_t ctrl_act_id)
{
  e2sm_rc_ctrl_hdr_t dst = {0};

  if (hdr_frmt == FORMAT_1_E2SM_RC_CTRL_HDR) {
    dst.format = FORMAT_1_E2SM_RC_CTRL_HDR;
    dst.frmt_1 = gen_rc_ctrl_hdr_frmt_1(ue_id, ric_style_type, ctrl_act_id);
  } else {
    assert(0!=0 && "not implemented the fill func for this ctrl hdr frmt");
  }

  return dst;
}

typedef enum {
    RRM_Policy_Ratio_List_8_4_3_6 = 1,
    RRM_Policy_Ratio_Group_8_4_3_6 = 2,
    RRM_Policy_8_4_3_6 = 3,
    RRM_Policy_Member_List_8_4_3_6 = 4,
    RRM_Policy_Member_8_4_3_6 = 5,
    PLMN_Identity_8_4_3_6 = 6,
    S_NSSAI_8_4_3_6 = 7,
    SST_8_4_3_6 = 8,
    SD_8_4_3_6 = 9,
    Min_PRB_Policy_Ratio_8_4_3_6 = 10,
    Max_PRB_Policy_Ratio_8_4_3_6 = 11,
    Dedicated_PRB_Policy_Ratio_8_4_3_6 = 12,
} slice_level_PRB_quota_param_id_e;

static
void gen_rrm_policy_ratio_group(lst_ran_param_t* RRM_Policy_Ratio_Group,
                                const char* sst_str,
                                const char* sd_str,
                                int min_ratio_prb,
                                int dedicated_ratio_prb,
                                int max_ratio_prb)
{
  // RRM Policy Ratio Group, STRUCTURE (RRM Policy Ratio List -> RRM Policy Ratio Group)
  // lst_ran_param_t* RRM_Policy_Ratio_Group = &RRM_Policy_Ratio_List->ran_param_val.lst->lst_ran_param[0];
  // RRM_Policy_Ratio_Group->ran_param_id = RRM_Policy_Ratio_Group_8_4_3_6;
  RRM_Policy_Ratio_Group->ran_param_struct.sz_ran_param_struct = 4;
  RRM_Policy_Ratio_Group->ran_param_struct.ran_param_struct = calloc(4, sizeof(seq_ran_param_t));
  assert(RRM_Policy_Ratio_Group->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");
  // RRM Policy, STRUCTURE (RRM Policy Ratio Group -> RRM Policy)
  seq_ran_param_t* RRM_Policy = &RRM_Policy_Ratio_Group->ran_param_struct.ran_param_struct[0];
  RRM_Policy->ran_param_id = RRM_Policy_8_4_3_6;
  RRM_Policy->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  RRM_Policy->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(RRM_Policy->ran_param_val.strct != NULL && "Memory exhausted");
  RRM_Policy->ran_param_val.strct->sz_ran_param_struct = 1;
  RRM_Policy->ran_param_val.strct->ran_param_struct = calloc(1, sizeof(seq_ran_param_t));
  assert(RRM_Policy->ran_param_val.strct->ran_param_struct != NULL && "Memory exhausted");
  // RRM Policy Member List, LIST (RRM Policy -> RRM Policy Member List)
  seq_ran_param_t* RRM_Policy_Member_List = &RRM_Policy->ran_param_val.strct->ran_param_struct[0];
  RRM_Policy_Member_List->ran_param_id = RRM_Policy_Member_List_8_4_3_6;
  RRM_Policy_Member_List->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  RRM_Policy_Member_List->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(RRM_Policy_Member_List->ran_param_val.lst != NULL && "Memory exhausted");
  RRM_Policy_Member_List->ran_param_val.lst->sz_lst_ran_param = 1;
  RRM_Policy_Member_List->ran_param_val.lst->lst_ran_param = calloc(1, sizeof(lst_ran_param_t));
  assert(RRM_Policy_Member_List->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");
  // RRM Policy Member, STRUCTURE (RRM Policy Member List -> RRM Policy Member)
  lst_ran_param_t* RRM_Policy_Member = &RRM_Policy_Member_List->ran_param_val.lst->lst_ran_param[0];
  // RRM_Policy_Member->ran_param_id = RRM_Policy_Member_8_4_3_6;
  RRM_Policy_Member->ran_param_struct.sz_ran_param_struct = 2;
  RRM_Policy_Member->ran_param_struct.ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  assert(RRM_Policy_Member->ran_param_struct.ran_param_struct != NULL && "Memory exhausted");
  // PLMN Identity, ELEMENT (RRM Policy Member -> PLMN Identity)
  seq_ran_param_t* PLMN_Identity = &RRM_Policy_Member->ran_param_struct.ran_param_struct[0];
  PLMN_Identity->ran_param_id = PLMN_Identity_8_4_3_6;
  PLMN_Identity->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  PLMN_Identity->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(PLMN_Identity->ran_param_val.flag_false != NULL && "Memory exhausted");
  PLMN_Identity->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  char plmnid_str[] = "00101";
  byte_array_t plmn_id = cp_str_to_ba(plmnid_str); // TODO
  PLMN_Identity->ran_param_val.flag_false->octet_str_ran.len = plmn_id.len;
  PLMN_Identity->ran_param_val.flag_false->octet_str_ran.buf = plmn_id.buf;
  // S-NSSAI, STRUCTURE (RRM Policy Member -> S-NSSAI)
  seq_ran_param_t* S_NSSAI = &RRM_Policy_Member->ran_param_struct.ran_param_struct[1];
  S_NSSAI->ran_param_id = S_NSSAI_8_4_3_6;
  S_NSSAI->ran_param_val.type = STRUCTURE_RAN_PARAMETER_VAL_TYPE;
  S_NSSAI->ran_param_val.strct = calloc(1, sizeof(ran_param_struct_t));
  assert(S_NSSAI->ran_param_val.strct != NULL && "Memory exhausted");
  S_NSSAI->ran_param_val.strct->sz_ran_param_struct = 2;
  S_NSSAI->ran_param_val.strct->ran_param_struct = calloc(2, sizeof(seq_ran_param_t));
  // SST, ELEMENT (S-NSSAI -> SST)
  seq_ran_param_t* SST = &S_NSSAI->ran_param_val.strct->ran_param_struct[0];
  SST->ran_param_id = SST_8_4_3_6;
  SST->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  SST->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(SST->ran_param_val.flag_false != NULL && "Memory exhausted");
  SST->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  // char sst_str[] = "1";
  byte_array_t sst = cp_str_to_ba(sst_str); //TODO
  SST->ran_param_val.flag_false->octet_str_ran.len = sst.len;
  SST->ran_param_val.flag_false->octet_str_ran.buf = sst.buf;
  // SD, ELEMENT (S-NSSAI -> SD)
  seq_ran_param_t* SD = &S_NSSAI->ran_param_val.strct->ran_param_struct[1];
  SD->ran_param_id = SD_8_4_3_6;
  SD->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  SD->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(SD->ran_param_val.flag_false != NULL && "Memory exhausted");
  SD->ran_param_val.flag_false->type = OCTET_STRING_RAN_PARAMETER_VALUE;
  // char sd_str[] = "0";
  byte_array_t sd = cp_str_to_ba(sd_str); //TODO
  SD->ran_param_val.flag_false->octet_str_ran.len = sd.len;
  SD->ran_param_val.flag_false->octet_str_ran.buf = sd.buf;
  // Min PRB Policy Ratio, ELEMENT (RRM Policy Ratio Group -> Min PRB Policy Ratio)
  seq_ran_param_t* Min_PRB_Policy_Ratio = &RRM_Policy_Ratio_Group->ran_param_struct.ran_param_struct[1];
  Min_PRB_Policy_Ratio->ran_param_id = Min_PRB_Policy_Ratio_8_4_3_6;
  Min_PRB_Policy_Ratio->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  Min_PRB_Policy_Ratio->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(Min_PRB_Policy_Ratio->ran_param_val.flag_false != NULL && "Memory exhausted");
  Min_PRB_Policy_Ratio->ran_param_val.flag_false->type = INTEGER_RAN_PARAMETER_VALUE;
  // TODO: not handle this value in OAI
  Min_PRB_Policy_Ratio->ran_param_val.flag_false->int_ran = min_ratio_prb;
  // Max PRB Policy Ratio, ELEMENT (RRM Policy Ratio Group -> Max PRB Policy Ratio)
  seq_ran_param_t* Max_PRB_Policy_Ratio = &RRM_Policy_Ratio_Group->ran_param_struct.ran_param_struct[2];
  Max_PRB_Policy_Ratio->ran_param_id = Max_PRB_Policy_Ratio_8_4_3_6;
  Max_PRB_Policy_Ratio->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  Max_PRB_Policy_Ratio->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(Max_PRB_Policy_Ratio->ran_param_val.flag_false != NULL && "Memory exhausted");
  Max_PRB_Policy_Ratio->ran_param_val.flag_false->type = INTEGER_RAN_PARAMETER_VALUE;
  // TODO: not handle this value in OAI
  Max_PRB_Policy_Ratio->ran_param_val.flag_false->int_ran = max_ratio_prb;
  // Dedicated PRB Policy Ratio, ELEMENT (RRM Policy Ratio Group -> Dedicated PRB Policy Ratio)
  seq_ran_param_t* Dedicated_PRB_Policy_Ratio = &RRM_Policy_Ratio_Group->ran_param_struct.ran_param_struct[3];
  Dedicated_PRB_Policy_Ratio->ran_param_id = Dedicated_PRB_Policy_Ratio_8_4_3_6;
  Dedicated_PRB_Policy_Ratio->ran_param_val.type = ELEMENT_KEY_FLAG_FALSE_RAN_PARAMETER_VAL_TYPE;
  Dedicated_PRB_Policy_Ratio->ran_param_val.flag_false = calloc(1, sizeof(ran_parameter_value_t));
  assert(Dedicated_PRB_Policy_Ratio->ran_param_val.flag_false != NULL && "Memory exhausted");
  Dedicated_PRB_Policy_Ratio->ran_param_val.flag_false->type = INTEGER_RAN_PARAMETER_VALUE;
  Dedicated_PRB_Policy_Ratio->ran_param_val.flag_false->int_ran = dedicated_ratio_prb;

  return;
}

static
void gen_rrm_policy_ratio_list(seq_ran_param_t* RRM_Policy_Ratio_List)
{
  int num_slice = 3;    //peihao change the number of slices here
  // seq_ran_param_t* RRM_Policy_Ratio_List =  &dst.ran_param[0];
  RRM_Policy_Ratio_List->ran_param_id = RRM_Policy_Ratio_List_8_4_3_6;
  RRM_Policy_Ratio_List->ran_param_val.type = LIST_RAN_PARAMETER_VAL_TYPE;
  RRM_Policy_Ratio_List->ran_param_val.lst = calloc(1, sizeof(ran_param_list_t));
  assert(RRM_Policy_Ratio_List->ran_param_val.lst != NULL && "Memory exhausted");
  RRM_Policy_Ratio_List->ran_param_val.lst->sz_lst_ran_param = num_slice;
  RRM_Policy_Ratio_List->ran_param_val.lst->lst_ran_param = calloc(num_slice, sizeof(lst_ran_param_t));
  assert(RRM_Policy_Ratio_List->ran_param_val.lst->lst_ran_param != NULL && "Memory exhausted");

  int numbers[4];
  while(1){
  //for(int i = 0; i<2; i++){
    FILE *file = fopen("flexric/trandata/slice_ctrl.bin", "rb+");
    
    if (!file) {
      perror("Unable to open slice_ctrl.bin file \n");
      return EXIT_FAILURE;
    }
  
    if (fread(numbers, sizeof(int), 4, file) != 4) {
        perror("Failed to read integers from file");
        fclose(file);
        return EXIT_FAILURE;
    }
    
    // printf("number %d number %d number %d\n", numbers[0], numbers[1], numbers[2]);
    
    if(numbers[3] == 0){    //for real system
    //if (true){  //for simulation
      /*
      numbers[2] = 1;
      fseek(file, -sizeof(int), SEEK_CUR);
      if (fwrite(&numbers[2], sizeof(int), 1, file) != 1) {
            perror("Failed to write integer to file");
            fclose(file);
            return EXIT_FAILURE;
        }
      printf("number %d number %d number %d\n", numbers[0], numbers[1], numbers[2]);
      */
      //printf("Third integer was 0, changed to 1 and written to the file.\n");
      //printf("number %d number %d number %d\n", numbers[0], numbers[1], numbers[2]);
      //printf("number %d number %d number %d\n", numbers[0], numbers[1], numbers[2]);
      fclose(file);
      break;
    } else {
        fclose(file);
        continue;
        printf("Third integer is not 0, no changes made.\n");
    }

      fclose(file);
  }


  const char* sst_str[] = {"1", "1", "1"};
  const char* sd_str[] = {"1", "5", "9"};
  int dedicated_ratio_prb[] = {numbers[0], numbers[1], numbers[2]};
  int Max_PRB_Ratio[] = {80, 80, 80};
  int Min_PRB_Ratio[] = {20, 20, 80};
  for (int i = 0; i < num_slice; i++) {
    gen_rrm_policy_ratio_group(&RRM_Policy_Ratio_List->ran_param_val.lst->lst_ran_param[i],
                               sst_str[i],
                               sd_str[i],
                               Min_PRB_Ratio[i],
                               dedicated_ratio_prb[i], 
                               Max_PRB_Ratio[i]);
  }

  return;
}

static
e2sm_rc_ctrl_msg_frmt_1_t gen_rc_ctrl_msg_frmt_1_slice_level_PRB_quota()
{
  e2sm_rc_ctrl_msg_frmt_1_t dst = {0};

  // 8.4.3.6
  // RRM Policy Ratio List, LIST (len 1)
  // > RRM Policy Ratio Group, STRUCTURE (len 4)
  // >>  RRM Policy, STRUCTURE (len 1)
  // >>> RRM Policy Member List, LIST (len 1)
  // >>>> RRM Policy Member, STRUCTURE (len 2)
  // >>>>> PLMN Identity, ELEMENT
  // >>>>> S-NSSAI, STRUCTURE (len 2)
  // >>>>>> SST, ELEMENT
  // >>>>>> SD, ELEMENT
  // >> Min PRB Policy Ratio, ELEMENT
  // >> Max PRB Policy Ratio, ELEMENT
  // >> Dedicated PRB Policy Ratio, ELEMENT


  // RRM Policy Ratio List, LIST
  dst.sz_ran_param = 1;
  dst.ran_param = calloc(1, sizeof(seq_ran_param_t));
  assert(dst.ran_param != NULL && "Memory exhausted");
  gen_rrm_policy_ratio_list(&dst.ran_param[0]);

  return dst;
}

static
e2sm_rc_ctrl_msg_t gen_rc_ctrl_msg(e2sm_rc_ctrl_msg_e msg_frmt)
{
  e2sm_rc_ctrl_msg_t dst = {0};

  if (msg_frmt == FORMAT_1_E2SM_RC_CTRL_MSG) {
    dst.format = msg_frmt;
    dst.frmt_1 = gen_rc_ctrl_msg_frmt_1_slice_level_PRB_quota();
  } else {
    assert(0!=0 && "not implemented the fill func for this ctrl msg frmt");
  }

  return dst;
}

char mode0[] = "w+";

static
ue_id_e2sm_t gen_rc_ue_id(ue_id_e2sm_e type)
{
  ue_id_e2sm_t ue_id = {0};
  if (type == GNB_UE_ID_E2SM) {
    ue_id.type = GNB_UE_ID_E2SM;
    // TODO
    ue_id.gnb.amf_ue_ngap_id = 0;
    ue_id.gnb.guami.plmn_id.mcc = 1;
    ue_id.gnb.guami.plmn_id.mnc = 1;
    ue_id.gnb.guami.plmn_id.mnc_digit_len = 2;
    ue_id.gnb.guami.amf_region_id = 0;
    ue_id.gnb.guami.amf_set_id = 0;
    ue_id.gnb.guami.amf_ptr = 0;
  } else {
    assert(0!=0 && "not supported UE ID type");
  }
  return ue_id;
}
//*******************************************SLICE END**********************************

//*******************************************KPM BEGIN***************************************
static
uint64_t const period_ms = 10;

static
pthread_mutex_t mtx;

//std::ofstream 
FILE *fout_kpm;
char filename[] = "flexric/trandata/KPM_UE.txt";
char mode[] = "a+";


struct kpm1 {
    uint64_t id;
    uint32_t PdcpSduVolumeDL;
    uint32_t PdcpSduVolumeUL;
    double RlcSduDelayDl;
    double UEThpDl;
    double UEThpUL;
    uint32_t PrbTotDl;
    uint32_t PrbTotUl;
} kpm_data;

int i = 0;
int num_line = 0;
    
static
void log_gnb_ue_id(ue_id_e2sm_t ue_id)
{
  i++;
  //printf("###############the totle number of lines in the file%d", i);
  /*if (i>2){
    fout_kpm = fopen(filename, mode0);
    i = 0;
  }else{
    fout_kpm = fopen(filename, mode);
  }*/
  if (num_line < 5){
    fout_kpm = fopen(filename, mode);
    num_line ++;
  }else{
    fout_kpm = fopen(filename, mode0);
    num_line = 0;
  } 

  if (ue_id.gnb.gnb_cu_ue_f1ap_lst != NULL) {
    for (size_t i = 0; i < ue_id.gnb.gnb_cu_ue_f1ap_lst_len; i++) {
      //printf("UE ID type = gNB-CU, gnb_cu_ue_f1ap = %u\n", ue_id.gnb.gnb_cu_ue_f1ap_lst[i]);
    }
  } else {
    //printf("UE ID type = gNB, amf_ue_ngap_id = %lu\n", ue_id.gnb.amf_ue_ngap_id);
    //printf("111#####################\n");
  }
  if (ue_id.gnb.ran_ue_id != NULL) {
    //printf("ran_ue_id = %lx", *ue_id.gnb.ran_ue_id); // RAN UE NGAP ID
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%ld ", *ue_id.gnb.ran_ue_id);
    fclose(fout_kpm);
  }
}

static
void log_du_ue_id(ue_id_e2sm_t ue_id)
{
  fout_kpm = fopen(filename, mode0);
  //printf("UE ID type = gNB-DU, gnb_cu_ue_f1ap = %u\n", ue_id.gnb_du.gnb_cu_ue_f1ap);
  if (ue_id.gnb_du.ran_ue_id != NULL) {
    //printf("ran_ue_id = %lx\n", *ue_id.gnb_du.ran_ue_id); // RAN UE NGAP ID
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%ld ", *ue_id.gnb_du.ran_ue_id);
    fclose(fout_kpm);
  }
}

static
void log_cuup_ue_id(ue_id_e2sm_t ue_id)
{
  fout_kpm = fopen(filename, mode0);
  //printf("UE ID type = gNB-CU-UP, gnb_cu_cp_ue_e1ap = %u\n", ue_id.gnb_cu_up.gnb_cu_cp_ue_e1ap);
  if (ue_id.gnb_cu_up.ran_ue_id != NULL) {
    //printf("ran_ue_id = %lx\n", *ue_id.gnb_cu_up.ran_ue_id); // RAN UE NGAP ID
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%ld ", *ue_id.gnb_cu_up.ran_ue_id);
    fclose(fout_kpm);
  }
}

typedef void (*log_ue_id)(ue_id_e2sm_t ue_id);

static
log_ue_id log_ue_id_e2sm[END_UE_ID_E2SM] = {
    log_gnb_ue_id, // common for gNB-mono, CU and CU-CP
    log_du_ue_id,
    log_cuup_ue_id,
    NULL,
    NULL,
    NULL,
    NULL,
};

static
void log_int_value(byte_array_t name, meas_record_lst_t meas_record)
{
  fout_kpm = fopen(filename, mode);
  if (cmp_str_ba("RRU.PrbTotDl", name) == 0) {
    //printf("RRU.PrbTotDl = %d [PRBs]\n", meas_record.int_val);
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%d ", meas_record.int_val);
    fclose(fout_kpm);
  } else if (cmp_str_ba("RRU.PrbTotUl", name) == 0) {
    //printf("RRU.PrbTotUl = %d [PRBs]\n", meas_record.int_val);
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%d\n", meas_record.int_val);
    fclose(fout_kpm);
  } else if (cmp_str_ba("DRB.PdcpSduVolumeDL", name) == 0) {
    //printf("DRB.PdcpSduVolumeDL = %d [kb]\n", meas_record.int_val);
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%d ", meas_record.int_val);
    fclose(fout_kpm);
  } else if (cmp_str_ba("DRB.PdcpSduVolumeUL", name) == 0) {
    //printf("DRB.PdcpSduVolumeUL = %d [kb]\n", meas_record.int_val);
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%d ", meas_record.int_val);
    fclose(fout_kpm);
  } else {
    printf("Measurement Name not yet supported\n");
  }
}

static
void log_real_value(byte_array_t name, meas_record_lst_t meas_record)
{
  fout_kpm = fopen(filename, mode);
  if (cmp_str_ba("DRB.RlcSduDelayDl", name) == 0) {
    //printf("DRB.RlcSduDelayDl = %.2f [μs]\n", meas_record.real_val);
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%.2f ", meas_record.real_val);
    fclose(fout_kpm);
  } else if (cmp_str_ba("DRB.UEThpDl", name) == 0) {
    //printf("DRB.UEThpDl = %.2f [kbps]\n", meas_record.real_val);
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%.2f ", meas_record.real_val);
    fclose(fout_kpm);
  } else if (cmp_str_ba("DRB.UEThpUl", name) == 0) {
    //printf("DRB.UEThpUl = %.2f [kbps]\n", meas_record.real_val);
    fseek(fout_kpm, 0, SEEK_SET);
    fprintf(fout_kpm, "%.2f ", meas_record.real_val);
    fclose(fout_kpm);
  } else {
    printf("Measurement Name not yet supported\n");
  }
}

typedef void (*log_meas_value)(byte_array_t name, meas_record_lst_t meas_record);

static
log_meas_value get_meas_value[END_MEAS_VALUE] = {
    log_int_value,
    log_real_value,
    NULL,
};

static
void match_meas_name_type(meas_type_t meas_type, meas_record_lst_t meas_record)
{
  // Get the value of the Measurement
  get_meas_value[meas_record.value](meas_type.name, meas_record);
}

static
void match_id_meas_type(meas_type_t meas_type, meas_record_lst_t meas_record)
{
  (void)meas_type;
  (void)meas_record;
  assert(false && "ID Measurement Type not yet supported");
}

typedef void (*check_meas_type)(meas_type_t meas_type, meas_record_lst_t meas_record);

static
check_meas_type match_meas_type[END_MEAS_TYPE] = {
    match_meas_name_type,
    match_id_meas_type,
};

static
void log_kpm_measurements(kpm_ind_msg_format_1_t const* msg_frm_1)
{
  assert(msg_frm_1->meas_info_lst_len > 0 && "Cannot correctly print measurements");

  // UE Measurements per granularity period
  for (size_t j = 0; j < msg_frm_1->meas_data_lst_len; j++) {
    meas_data_lst_t const data_item = msg_frm_1->meas_data_lst[j];

    for (size_t z = 0; z < data_item.meas_record_len; z++) {
      meas_type_t const meas_type = msg_frm_1->meas_info_lst[z].meas_type;
      meas_record_lst_t const record_item = data_item.meas_record_lst[z];

      match_meas_type[meas_type.type](meas_type, record_item);

      if (data_item.incomplete_flag && *data_item.incomplete_flag == TRUE_ENUM_VALUE)
        printf("Measurement Record not reliable");
    }
  }

}

static
void sm_cb_kpm(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == KPM_STATS_V3_0);

  // Reading Indication Message Format 3
  kpm_ind_data_t const* ind = &rd->ind.kpm.ind;
  kpm_ric_ind_hdr_format_1_t const* hdr_frm_1 = &ind->hdr.kpm_ric_ind_hdr_format_1;
  kpm_ind_msg_format_3_t const* msg_frm_3 = &ind->msg.frm_3;

  int64_t const now = time_now_us();
  static int counter = 1;
  {
    lock_guard(&mtx);

    //printf("\n%7d KPM ind_msg latency = %ld [μs]\n", counter, now - hdr_frm_1->collectStartTime); // xApp <-> E2 Node

    // Reported list of measurements per UE
    for (size_t i = 0; i < msg_frm_3->ue_meas_report_lst_len; i++) {
      // log UE ID
      ue_id_e2sm_t const ue_id_e2sm = msg_frm_3->meas_report_per_ue[i].ue_meas_report_lst;
      ue_id_e2sm_e const type = ue_id_e2sm.type;
      log_ue_id_e2sm[type](ue_id_e2sm);

      // log measurements
      log_kpm_measurements(&msg_frm_3->meas_report_per_ue[i].ind_msg_format_1);
      
    }
    counter++;
  }
}

static
test_info_lst_t filter_predicate(test_cond_type_e type, test_cond_e cond, int value)
{
  test_info_lst_t dst = {0};

  dst.test_cond_type = type;
  // It can only be TRUE_TEST_COND_TYPE so it does not matter the type
  // but ugly ugly...
  dst.S_NSSAI = TRUE_TEST_COND_TYPE;

  dst.test_cond = calloc(1, sizeof(test_cond_e));
  assert(dst.test_cond != NULL && "Memory exhausted");
  *dst.test_cond = cond;

  dst.test_cond_value = calloc(1, sizeof(test_cond_value_t));
  assert(dst.test_cond_value != NULL && "Memory exhausted");
  dst.test_cond_value->type = OCTET_STRING_TEST_COND_VALUE;

  dst.test_cond_value->octet_string_value = calloc(1, sizeof(byte_array_t));
  assert(dst.test_cond_value->octet_string_value != NULL && "Memory exhausted");
  const size_t len_nssai = 1;
  dst.test_cond_value->octet_string_value->len = len_nssai;
  dst.test_cond_value->octet_string_value->buf = calloc(len_nssai, sizeof(uint8_t));
  assert(dst.test_cond_value->octet_string_value->buf != NULL && "Memory exhausted");
  dst.test_cond_value->octet_string_value->buf[0] = value;

  return dst;
}

static
label_info_lst_t fill_kpm_label(void)
{
  label_info_lst_t label_item = {0};

  label_item.noLabel = ecalloc(1, sizeof(enum_value_e));
  *label_item.noLabel = TRUE_ENUM_VALUE;

  return label_item;
}

static
kpm_act_def_format_1_t fill_act_def_frm_1(ric_report_style_item_t const* report_item)
{
  assert(report_item != NULL);

  kpm_act_def_format_1_t ad_frm_1 = {0};

  size_t const sz = report_item->meas_info_for_action_lst_len;

  // [1, 65535]
  ad_frm_1.meas_info_lst_len = sz;
  ad_frm_1.meas_info_lst = calloc(sz, sizeof(meas_info_format_1_lst_t));
  assert(ad_frm_1.meas_info_lst != NULL && "Memory exhausted");

  for (size_t i = 0; i < sz; i++) {
    meas_info_format_1_lst_t* meas_item = &ad_frm_1.meas_info_lst[i];
    // 8.3.9
    // Measurement Name
    meas_item->meas_type.type = NAME_MEAS_TYPE;
    meas_item->meas_type.name = copy_byte_array(report_item->meas_info_for_action_lst[i].name);

    // [1, 2147483647]
    // 8.3.11
    meas_item->label_info_lst_len = 1;
    meas_item->label_info_lst = ecalloc(1, sizeof(label_info_lst_t));
    meas_item->label_info_lst[0] = fill_kpm_label();
  }

  // 8.3.8 [0, 4294967295]
  ad_frm_1.gran_period_ms = period_ms;

  // 8.3.20 - OPTIONAL
  ad_frm_1.cell_global_id = NULL;

#if defined KPM_V2_03 || defined KPM_V3_00
  // [0, 65535]
  ad_frm_1.meas_bin_range_info_lst_len = 0;
  ad_frm_1.meas_bin_info_lst = NULL;
#endif

  return ad_frm_1;
}

static
kpm_act_def_t fill_report_style_4(ric_report_style_item_t const* report_item)
{
  assert(report_item != NULL);
  assert(report_item->act_def_format_type == FORMAT_4_ACTION_DEFINITION);

  kpm_act_def_t act_def = {.type = FORMAT_4_ACTION_DEFINITION};

  // Fill matching condition
  // [1, 32768]
  act_def.frm_4.matching_cond_lst_len = 1;
  act_def.frm_4.matching_cond_lst = calloc(act_def.frm_4.matching_cond_lst_len, sizeof(matching_condition_format_4_lst_t));
  assert(act_def.frm_4.matching_cond_lst != NULL && "Memory exhausted");
  // Filter connected UEs by S-NSSAI criteria
  test_cond_type_e const type = S_NSSAI_TEST_COND_TYPE; // CQI_TEST_COND_TYPE
  test_cond_e const condition = EQUAL_TEST_COND; // GREATERTHAN_TEST_COND
  int const value = 1;
  act_def.frm_4.matching_cond_lst[0].test_info_lst = filter_predicate(type, condition, value);

  // Fill Action Definition Format 1
  // 8.2.1.2.1
  act_def.frm_4.action_def_format_1 = fill_act_def_frm_1(report_item);

  return act_def;
}

typedef kpm_act_def_t (*fill_kpm_act_def)(ric_report_style_item_t const* report_item);

static
fill_kpm_act_def get_kpm_act_def[END_RIC_SERVICE_REPORT] = {
    NULL,
    NULL,
    NULL,
    fill_report_style_4,
    NULL,
};

static
kpm_sub_data_t gen_kpm_subs(kpm_ran_function_def_t const* ran_func)
{
  assert(ran_func != NULL);
  assert(ran_func->ric_event_trigger_style_list != NULL);

  kpm_sub_data_t kpm_sub = {0};

  // Generate Event Trigger
  assert(ran_func->ric_event_trigger_style_list[0].format_type == FORMAT_1_RIC_EVENT_TRIGGER);
  kpm_sub.ev_trg_def.type = FORMAT_1_RIC_EVENT_TRIGGER;
  kpm_sub.ev_trg_def.kpm_ric_event_trigger_format_1.report_period_ms = period_ms;

  // Generate Action Definition
  kpm_sub.sz_ad = 1;
  kpm_sub.ad = calloc(kpm_sub.sz_ad, sizeof(kpm_act_def_t));
  assert(kpm_sub.ad != NULL && "Memory exhausted");

  // Multiple Action Definitions in one SUBSCRIPTION message is not supported in this project
  // Multiple REPORT Styles = Multiple Action Definition = Multiple SUBSCRIPTION messages
  ric_report_style_item_t* const report_item = &ran_func->ric_report_style_list[0];
  ric_service_report_e const report_style_type = report_item->report_style_type;
  *kpm_sub.ad = get_kpm_act_def[report_style_type](report_item);

  return kpm_sub;
}

static
bool eq_sm(sm_ran_function_t const* elem, int const id)
{
  if (elem->id == id)
    return true;

  return false;
}

static
size_t find_sm_idx(sm_ran_function_t* rf, size_t sz, bool (*f)(sm_ran_function_t const*, int const), int const id)
{
  for (size_t i = 0; i < sz; i++) {
    if (f(&rf[i], id))
      return i;
  }

  assert(0 != 0 && "SM ID could not be found in the RAN Function List");
}
//*********************************************************KPM END*********************************

//*********************************************************MAC BEGIN**************************************
static
uint64_t cnt_mac;

static
void sm_cb_mac(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == MAC_STATS_V0);
 
  int64_t now = time_now_us();
  if(cnt_mac % 10240 == 0)
    printf("MAC ind_msg latency = %ld μs\n", now - rd->ind.mac.msg.tstamp);
  
  cnt_mac++;
}

static
uint64_t cnt_rlc;

static
void sm_cb_rlc(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);

  assert(rd->ind.type == RLC_STATS_V0);

  int64_t now = time_now_us();

  if(cnt_rlc % 1024 == 0)
    //printf("RLC ind_msg latency = %ld μs\n", now - rd->ind.rlc.msg.tstamp);
  cnt_rlc++;
}

static
uint64_t cnt_pdcp;

static
void sm_cb_pdcp(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);

  assert(rd->ind.type == PDCP_STATS_V0);

  int64_t now = time_now_us();

  if(cnt_pdcp % 1024 == 0)
    //printf("PDCP ind_msg latency = %ld μs\n", now - rd->ind.pdcp.msg.tstamp);

  cnt_pdcp++;
}

static
uint64_t cnt_gtp;

static
void sm_cb_gtp(sm_ag_if_rd_t const* rd)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);

  assert(rd->ind.type == GTP_STATS_V0);

  int64_t now = time_now_us();
  if(cnt_gtp % 1024 == 0)
    //printf("GTP ind_msg latency = %ld μs\n", now - rd->ind.gtp.msg.tstamp);

  cnt_gtp++;
}

//**************************************************MAC END*********************************************

int main(int argc, char *argv[])
{
  fr_args_t args = init_fr_args(argc, argv);
  //defer({ free_fr_args(&args); });

  //Init the xApp
  init_xapp_api(&args);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });

  assert(nodes.len > 0);
  printf("[KPM MAC SLICE]: Connected E2 nodes = %d\n", nodes.len);

//********************************************KPM BEGAIN**************************************
  pthread_mutexattr_t attr = {0};
  int rc = pthread_mutex_init(&mtx, &attr);
  assert(rc == 0);

  sm_ans_xapp_t* hndl = calloc(nodes.len, sizeof(sm_ans_xapp_t));
  assert(hndl != NULL);

  int const KPM_ran_function = 2;
 
  char mode0[] = "w+";
  fout_kpm = fopen(filename, mode0);
  if (fout_kpm == NULL) { 
    printf("cannot open the file\n");
    return 1;
  }
  printf("Successfull open the file\n");
  //fprintf(fout_kpm, "Hi Peihao!\n");

  //fprintf(fout_kpm, "Hi Peihao!\n");
  //*********************************************KPM END *******************************
  
 // MAC indication 
 
  const char* i_0 = "1_ms";
  sm_ans_xapp_t* mac_handle = NULL;
  // RLC indication
  const char* i_1 = "1_ms";
  sm_ans_xapp_t* rlc_handle = NULL;
  // PDCP indication
  const char* i_2 = "1_ms";
  sm_ans_xapp_t* pdcp_handle = NULL;
  // GTP indication
  const char* i_3 = "1_ms";
  sm_ans_xapp_t* gtp_handle = NULL;

  if(nodes.len > 0){
    mac_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(mac_handle  != NULL);
    rlc_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(rlc_handle  != NULL);
    pdcp_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(pdcp_handle  != NULL);
    gtp_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(gtp_handle  != NULL);
  }
  
  ////////////
  // START MAC
  ////////////

  for (int i = 0; i < nodes.len; i++) {
    e2_node_connected_xapp_t* n = &nodes.n[i];

    //*********************************************MAC ENGIN *******************************
    for (size_t j = 0; j < n->len_rf; j++)
      printf("Registered node %d ran func id = %d \n ", i, n->rf[j].id);

    if(n->id.type == ngran_gNB || n->id.type == ngran_eNB){
      // MAC Control is not yet implemented in OAI RAN
      // mac_ctrl_req_data_t wr = {.hdr.dummy = 1, .msg.action = 42 };
      // sm_ans_xapp_t const a = control_sm_xapp_api(&nodes.n[i].id, 142, &wr);
      // assert(a.success == true);

      mac_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 142, (void*)i_0, sm_cb_mac);
      assert(mac_handle[i].success == true);
      
      /*
      rlc_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 143, (void*)i_1, sm_cb_rlc);
      assert(rlc_handle[i].success == true);

      pdcp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 144, (void*)i_2, sm_cb_pdcp);
      assert(pdcp_handle[i].success == true);

      gtp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 148, (void*)i_3, sm_cb_gtp);
      assert(gtp_handle[i].success == true);
      */

    } else if(n->id.type ==  ngran_gNB_CU || n->id.type ==  ngran_gNB_CUUP){
      pdcp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 144, (void*)i_2, sm_cb_pdcp);
      assert(pdcp_handle[i].success == true);

      gtp_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 148, (void*)i_3, sm_cb_gtp);
      assert(gtp_handle[i].success == true);

    } else if(n->id.type == ngran_gNB_DU){
      mac_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 142, (void*)i_0, sm_cb_mac);
      assert(mac_handle[i].success == true);

      // rlc_handle[i] = report_sm_xapp_api(&nodes.n[i].id, 143, (void*)i_1, sm_cb_rlc);
      // assert(rlc_handle[i].success == true);
    }
    //*********************************************MAC END**********************************

    //********************************************KPM BEGIN*********************************

    size_t const idx = find_sm_idx(n->rf, n->len_rf, eq_sm, KPM_ran_function);
    assert(n->rf[idx].defn.type == KPM_RAN_FUNC_DEF_E && "KPM is not the received RAN Function");
    // if REPORT Service is supported by E2 node, send SUBSCRIPTION
    // e.g. OAI CU-CP
    //if (n->rf[idx].defn.kpm.ric_report_style_list != NULL) {
     // Generate KPM SUBSCRIPTION message
     kpm_sub_data_t kpm_sub = gen_kpm_subs(&n->rf[idx].defn.kpm);

     hndl[i] = report_sm_xapp_api(&n->id, KPM_ran_function, &kpm_sub, sm_cb_kpm);
     assert(hndl[i].success == true);
     //printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
     free_kpm_sub_data(&kpm_sub);

  //}
  //***************************************************KPM END****************************
  }

  ////////////
  // START RC
  ////////////

  // RC Control
  // CONTROL Service Style 2: Radio Resource Allocation Control
  // Action ID 6: Slice-level PRB quota
  // E2SM-RC Control Header Format 1
  // E2SM-RC Control Message Format 1
  while(1){
    rc_ctrl_req_data_t rc_ctrl = {0};
    ue_id_e2sm_t ue_id = gen_rc_ue_id(GNB_UE_ID_E2SM);

    rc_ctrl.hdr = gen_rc_ctrl_hdr(FORMAT_1_E2SM_RC_CTRL_HDR, ue_id, 2, Slice_level_PRB_quotal_7_6_3_1);
    rc_ctrl.msg = gen_rc_ctrl_msg(FORMAT_1_E2SM_RC_CTRL_MSG);

    int64_t st = time_now_us();
    for(size_t i =0; i < nodes.len; ++i){
      control_sm_xapp_api(&nodes.n[i].id, SM_RC_ID, &rc_ctrl);
    }


    int numbers[4];
    while(1){
      FILE *file = fopen("flexric/trandata/slice_ctrl.bin", "rb+");
      if (!file) {
        perror("Unable to open slice_ctrl.bin file \n");
        return EXIT_FAILURE;
      }
      if (fread(numbers, sizeof(int), 4, file) != 4) {
          perror("Failed to read integers from file");
          fclose(file);
          return EXIT_FAILURE;
      }
      if(numbers[3] == 0){
        numbers[3] = 1;
        fseek(file, -sizeof(int), SEEK_CUR);
        if (fwrite(&numbers[3], sizeof(int), 1, file) != 1) {
          perror("Failed to write integer to file");
          fclose(file);
          return EXIT_FAILURE;
        }
        //printf("number %d number %d number %d\n", numbers[0], numbers[1], numbers[2]);
        fclose(file);
        break;
      } else {
        fclose(file);
        continue;
        printf("Third integer is not 0, no changes made.\n");
      }
      fclose(file);
    }


    //printf("[xApp]: Control Loop Latency: %ld us\n", time_now_us() - st);
    free_rc_ctrl_req_data(&rc_ctrl);
    //sleep(1);
  }
  ////////////
  // END RC
  ////////////

  //free MAC
  for(int i = 0; i < nodes.len; ++i){
    // Remove the handle previously returned
    if(mac_handle[i].u.handle != 0 )
      rm_report_sm_xapp_api(mac_handle[i].u.handle);
    /*
    if(rlc_handle[i].u.handle != 0) 
      rm_report_sm_xapp_api(rlc_handle[i].u.handle);
    if(pdcp_handle[i].u.handle != 0)
      rm_report_sm_xapp_api(pdcp_handle[i].u.handle);
    if(gtp_handle[i].u.handle != 0)
      rm_report_sm_xapp_api(gtp_handle[i].u.handle);
    */
  }

  if(nodes.len > 0){
    free(mac_handle);
    /*
    free(rlc_handle);
    free(pdcp_handle);
    free(gtp_handle);
    */
  }
  
  //free monitor
  for (int i = 0; i < nodes.len; ++i) {
    // Remove the handle previously returned
    if (hndl[i].success == true)
      rm_report_sm_xapp_api(hndl[i].u.handle);
  }
  free(hndl);
  
  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");

}

