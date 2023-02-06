#include "global.h"
#include "stackful_coroutine.h"
#include "task.h"

static void Task_Coroutine(u8 taskId)
{
    s32 i, n;
    struct Coroutine **cos;

    cos = (struct Coroutine **)&gTasks[taskId].data;
    for (i = n = 0; i < sizeof(gTasks[taskId].data) / sizeof(*cos); i++)
    {
        if (cos[i] == NULL)
            break;

        if ((cos[n] = ResumeCoroutine(cos[i])))
            n++;
    }

    if (n == 0)
        DestroyTask(taskId);
}

void CreateCoroutineTask(struct Coroutine *co, u8 priority)
{
    s32 taskId, i;
    struct Coroutine **cos;

    for (taskId = 0; taskId < NUM_TASKS; taskId++)
    {
        if (gTasks[taskId].func == Task_Coroutine
         && gTasks[taskId].priority == priority)
        {
            cos = (struct Coroutine **)&gTasks[taskId].data;
            for (i = 0; i < sizeof(gTasks[taskId].data) / sizeof(*cos); i++)
            {
                if (cos[i] == NULL)
                {
                    cos[i] = co;
                    return;
                }
            }
        }
    }

    taskId = CreateTask(Task_Coroutine, priority);
    cos = (struct Coroutine **)&gTasks[taskId].data;
    cos[0] = co;
}
