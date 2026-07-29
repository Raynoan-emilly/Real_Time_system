#ifndef TASKS_H
#define TASKS_H

#ifdef __cplusplus
extern "C" {
#endif

void vTaskGenerator(void *pvParameters);
void vTaskCranes(void *pvParameters);
void vTaskTrucks(void *pvParameters);
void vTaskDashboard(void *pvParameters);
void vTaskLogger(void *pvParameters);
void vTaskButtons(void *pvParameters); 

#ifdef __cplusplus
}
#endif

#endif // TASKS_H