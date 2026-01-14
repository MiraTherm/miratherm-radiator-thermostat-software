#ifndef CORE_INC_SYSTEM_TASK_H
#define CORE_INC_SYSTEM_TASK_H

#include "cmsis_os2.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYSTEM_TASK_STACK_SIZE (512U * 4U)

typedef enum {
  STATE_INIT = 0,
  STATE_COD_DATE_TIME,
  STATE_COD_SCHEDULE,
  STATE_NOT_INST,
  STATE_ADAPT,
  STATE_ADAPT_FAIL,
  STATE_RUNNING,
  STATE_FACTORY_RST,
  STATE_MAINT /* Not implemented */
} SystemState_t;

typedef enum { MODE_AUTO = 0, MODE_MANUAL = 1, MODE_BOOST = 2 } SystemMode_t;

typedef enum {
  ADAPT_RESULT_UNKNOWN = -1,
  ADAPT_RESULT_OK = 0,
  ADAPT_RESULT_FAIL = 1
} AdaptResult_t;

typedef struct {
  SystemState_t state;
  SystemMode_t mode;              /* AUTO, MANUAL, or BOOST */
  SystemMode_t mode_before_boost; /* Mode before boost activation */
  uint32_t boost_begin_time;      /* Tick count when boost mode was activated */
  AdaptResult_t adapt_result;     /* ADAPT_RESULT_* */
  /* Target temperature and slot end time calculated in RUNNING state */
  float target_temp;
  uint8_t slot_end_hour;
  uint8_t slot_end_minute;
  /* Temporary target temperature override (set by rotary encoder, cleared when
   * slot expires) */
  float temporary_target_temp; /* 0 = not set, otherwise contains override
                                  temperature */
} SystemContextTypeDef;

typedef struct {
  osMutexId_t mutex;
  SystemContextTypeDef data;
} SystemContextAccessTypeDef;

/************************************************
 * ViewPresenter -> System events + args
 ************************************************/
typedef enum {
  EVT_NO_EVENT = 0,
  EVT_COD_DT_DONE,    /* Date/Time setup done */
  EVT_COD_SCH_DONE,   /* Schedule setup done */
  EVT_INST_REQ,       /* start adaptation */
  EVT_ADAPT_RST,      /* user accepts adapt fail and requests retry */
  EVT_FACTORY_RST_REQ /* Factory reset requested */
} VP2SystemEventTypeDef;

/************************************************
 * System -> ViewPresenter events
 ************************************************/
typedef enum {
  EVT_SYS_INIT_END =
      0 /* System initialization complete, UI can start rendering */
} System2VPEventTypeDef;

/* Forward declaration */
typedef struct ConfigAccessTypeDef ConfigAccessTypeDef;

/* Arguments passed to StartSystemTask via the thread create call */
typedef struct {
  osMessageQueueId_t vp2_system_queue;      /* ViewPresenter -> System */
  osMessageQueueId_t system2_vp_queue;      /* System -> ViewPresenter */
  osMessageQueueId_t system2_maint_queue;   /* System -> Maint */
  osMessageQueueId_t maint2_system_queue;   /* Maint -> System */
  osMessageQueueId_t system2_storage_queue; /* System -> Storage */
  SystemContextAccessTypeDef
      *system_context_access; /* Pointer to shared system context */
  ConfigAccessTypeDef
      *config_access; /* Pointer to configuration for schedule lookup */
} SystemTaskArgsTypeDef;

/* Thread function */
void StartSystemTask(void *argument);

/* Query current system state safely */
SystemState_t System_GetState(void);

/* Debug helpers: lock/unlock shared system context with logging */
bool System_LockContext(uint32_t timeout_ms);
void System_UnlockContext(void);

/* Query state machine current state (used internally) */
SystemState_t SystemSM_GetCurrentState(void);

#ifdef __cplusplus
}
#endif

#endif /* CORE_INC_SYSTEM_TASK_H */
