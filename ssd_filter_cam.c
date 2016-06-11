#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/bus.h>
#include <cam/cam.h>
#include <cam/cam_sim.h>
#include <cam/scsi/scsi_all.h>
#include <cam/cam_ccb.h>
#include <cam/scsi/scsi_pass.h>
#include <cam/cam_xpt.h>
#include <cam/cam_periph.h>

sim_action_func original_strategy_fn; 
int init_filter(struct module *module, int event, void *arg); 
int change_request_fn(void);
int restore_request_fn(void);
void my_filter_request(struct cam_sim *sim, union ccb *ccb);
struct cam_periph *find_periph_with_device(char *name, u_int unit);

void my_filter_request(struct cam_sim *sim, union ccb *ccb)
{
    // just verify its the same SIM.
   // ASSERT(cam_sim == sim );

    printf("entering the filter for the request ..\n");
    //modify ccb as you like. 
    //bio->bio_pblkno = -100;
    // MAke the original request.
    original_strategy_fn(sim, ccb);
 
    return ;
}

struct cam_periph *find_periph_with_device(char *name, u_int unit)
{ 
	struct cam_periph *periph;
        struct periph_driver **p_drv;

        /* Keep the list from changing while we traverse it */
        xpt_lock_buses();

        /* first find our driver in the list of drivers */
           for (p_drv = periph_drivers; *p_drv != NULL; p_drv++)
                if (strcmp((*p_drv)->driver_name, name) == 0)
                        break;

        if (*p_drv == NULL) {
             xpt_unlock_buses();
             return NULL;
        }

        /*
         * Run through every peripheral instance of this driver
         * and check to see whether it matches the unit passed
         * in by the user.  If it does, get out of the loops and
         * find the passthrough driver associated with that
         * peripheral driver.
         */
         for (periph = TAILQ_FIRST(&(*p_drv)->units); periph != NULL;
              periph = TAILQ_NEXT(periph, unit_links)) {

              if (periph->unit_number == unit)
                      break;
        }
        xpt_unlock_buses();

        // At the end here we have periph structure.
        return periph;
}

struct cam_sim *sim;
int change_request_fn(void)
{
   char path[10] = "pass";
   struct cam_periph *periph;


   // THe unit number in the second argument.
   periph = find_periph_with_device(path, 1);

   sim = periph->sim; 

   if(sim)
   {
       // get only the last sim_action
       original_strategy_fn = sim->sim_action;
       sim->sim_action = my_filter_request;
       printf("SUccess \n");
   }

   printf("change request fn \n"); 
   return 0;
}
 
int restore_request_fn(void)
{
    printf("restore requestfn \n"); 
    if(sim)
    sim->sim_action = original_strategy_fn;
    return 0;
}


int init_filter(struct module *module, int event, void *arg) 
{
    int ret = 0;

    switch(event)
    {
       case MOD_LOAD:
       {
          if((ret = change_request_fn()))
          {
            printf("Filter changing error ..\n");
          }
           break;
       }
       case MOD_UNLOAD:
       {

          restore_request_fn();
          printf("EXIT.\n");
          break;
       } 
       default:
           printf("default hanler");
    } 
    return 0;
}



/* The second argument of DECLARE_MODULE. */
moduledata_t moduleData = {
    "filter",    /* module name */
     init_filter,  /* event handler */
     NULL            /* extra data */
};
 
DECLARE_MODULE(filter, moduleData, SI_SUB_DRIVERS, SI_ORDER_MIDDLE); 
MODULE_DEPEND(filter, ad ,1,1,1);
