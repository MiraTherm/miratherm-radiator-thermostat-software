#ifndef CORE_INC_TASK_DEBUG_H
#define CORE_INC_TASK_DEBUG_H

/* Enable this to log thread creation/start details. Set to 0 to disable. */
#ifndef OS_TASKS_DEBUG
#define OS_TASKS_DEBUG 0
#endif

#ifndef ERROR_HANDLER_ON_FAILURE
#define ERROR_HANDLER_ON_FAILURE 0
#endif

#ifndef INPUT_TASK_DEBUG_PRINTING
#define INPUT_TASK_DEBUG_PRINTING 0
#endif

#ifndef SENSOR_TASK_DEBUG_PRINTING
#define SENSOR_TASK_DEBUG_PRINTING 0
#endif

#ifndef VIEW_PRESENTER_TASK_DEBUG_LEDS
#define VIEW_PRESENTER_TASK_DEBUG_LEDS 0
#endif

#endif /* CORE_INC_TASK_DEBUG_H */
