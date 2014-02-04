/* Common definitions for udpClient/Server */
/*   */
/* Tahir.Nawaz.Minhaz@bth.se */
/* Patrik.Arlos@bth.se */
/* */


typedef struct{
  u_int32_t exp_id;
  u_int32_t run_id;
  u_int32_t key_id;
  u_int32_t counter;
  u_int64_t starttime;
  u_int64_t stoptime;
  timeval depttime;
  char junk[1500];
}transfer_data;
