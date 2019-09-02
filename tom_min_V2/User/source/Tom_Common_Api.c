#include "Tom_Common_Api.h"



void get_task_info(TaskHandle_t task)
{
		eTaskState task_status;
		char *task_name;
		UBaseType_t stack_size;
		
		task_name = pcTaskGetTaskName(task);
		task_status = eTaskGetState(task);
		printf("current_task name: %s\r\n",task_name);
		printf("current_task status: %d\r\n",task_status);
		if(task_status == 0)
		{
				printf("%s task_status : eRunning\r\n",task_name);
		}	
		else if(task_status == 1)
		{
				printf("%s task_status : eReady\r\n",task_name);
		}
		else if(task_status == 2)
		{
				printf("%s task_status : eBlocked\r\n",task_name);
		}
		else if(task_status == 3)
		{
				printf("%s task_status : eSuspended\r\n",task_name);
		}
		else if(task_status == 4)
		{	
				printf("%s task_status : eDeleted\r\n",task_name);
		}
		else if(task_status == 5)
		{
				printf("%s task_status : eInvalid\r\n",task_name);
		}
		
		stack_size = uxTaskGetStackHighWaterMark(task);
		printf("Remaining stack space : %ld\r\n",stack_size*4);
		
}