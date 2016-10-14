#include <prussdrv.h>
#include <stdio.h>
#include <stdlib.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM0 0

main() {
  // addresses	
  unsigned int ret;
  
  // initialize pru

  tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;
  
  /* Initialize the PRU */
  if ((ret = prussdrv_init()) != 0) {
    fprintf(stderr, "pru_setup(): path is NULL\n");
    exit(-1);
  }
  
  /* Open PRU Interrupt */
  if ((ret = prussdrv_open(PRU_EVTOUT_0))) {
    printf("prussdrv_open open failed\n");
    return;
  }
  
  //printf("resetting\n");
  prussdrv_pru_reset(0);
  
  /* Get the interrupt initialized */
  if((ret = prussdrv_pruintc_init(&intc)) != 0) {
    fprintf(stderr, "prussdrv_pruintc_init() failed\n");
    exit(ret);
  }
  
  /* Initialize example */
  if ((ret = prussdrv_exec_program (PRU_NUM0, "./bin/stoplaser.bin")) < 0) {
    printf("prussdrv_exec_program() failed\n");
    exit(ret);
  }
}
