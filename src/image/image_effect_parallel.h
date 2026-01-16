#pragma once

_PHOXO_BEGIN
/// @cond
class ParallelTask
{
private:
    const CRect   m_task_region;
    ImageEffect   & m_effect;
    Image   & m_img;
    ATL::CHandle   m_finish_event;
    PTP_WORK   m_work = NULL;

public:
    ParallelTask(const CRect& task_region, ImageEffect& effect, Image& img)
        : m_task_region(task_region)
        , m_effect(effect)
        , m_img(img)
    {
    }

    ~ParallelTask() // not virtual
    {
        if (m_work)
        {
            WaitForThreadpoolWorkCallbacks(m_work, TRUE);
            CloseThreadpoolWork(m_work);
        }
    }

    HANDLE Execute()
    {
        m_finish_event.Attach(CreateEvent(NULL, TRUE, FALSE, NULL));
        m_work = CreateThreadpoolWork(execute_task_proc, this, NULL); assert(m_work);
        SubmitThreadpoolWork(m_work);
        return m_finish_event;
    }

private:
    void ProcessInThread()
    {
        m_effect.ProcessRegion(m_img, m_task_region, nullptr);
        ::SetEvent(m_finish_event);
    }

    static VOID CALLBACK execute_task_proc(PTP_CALLBACK_INSTANCE, PVOID Context, PTP_WORK)
    {
        ((ParallelTask*)Context)->ProcessInThread();
    }
};

class ParallelTaskExecutor
{
private:
    int   m_total = 0, m_finish = 0;
    std::deque<unique_ptr<ParallelTask>>   m_waiting_task;
    std::vector<unique_ptr<ParallelTask>>   m_running_task;
    std::vector<HANDLE>   m_running_finish_event;

public:
    ParallelTaskExecutor(Image& img, ImageEffect& effect)
    {
        int   cnt = GetMaxThreadCount();
        m_running_task.reserve(cnt);
        m_running_finish_event.reserve(cnt);
        SplitParallelTasks(img, effect);
        m_total = (int)m_waiting_task.size();
    }

    void ParallelExecuteTask(IProgressListener* progress)
    {
        for (;;)
        {
            RunWaitingTasks();
            if (m_running_task.empty())
                break;

            WaitFinishEvent(false); // wait only one task finish
            if (progress && !progress->UpdateProgress(100 * m_finish / m_total))
            {
                WaitFinishEvent(true); // wait all finish
                break;
            }
        }
    }

private:
    static int GetMaxThreadCount()
    {
        static int   v = std::clamp((int)GetActiveProcessorCount(ALL_PROCESSOR_GROUPS), 4, 48);
        return v;
    }

    void SplitParallelTasks(Image& img, ImageEffect& effect)
    {
        CRect   rc(0, 0, img.Width(), img.Height());
        const CSize   region = effect.GetScanLineCountPerTask(img);
        if (region.cx)
            rc.right = region.cx; // split horizontally from left to right
        else
            rc.bottom = region.cy; // split vertically from top to bottom

        for (;;)
        {
            rc.bottom = (std::min)((int)rc.bottom, img.Height());
            rc.right = (std::min)((int)rc.right, img.Width());
            m_waiting_task.push_back(make_unique<ParallelTask>(rc, effect, img));

            if ((rc.bottom == img.Height()) && (rc.right == img.Width()))
                break; // <--- the last task

            rc.OffsetRect(region);
        }
    }

    void RunWaitingTasks()
    {
        int   add = GetMaxThreadCount() - (int)m_running_task.size();
        for (int i = 0; i < add; i++)
        {
            RunFirstWaitingTask();
        }
    }

    void RunFirstWaitingTask()
    {
        if (m_waiting_task.size())
        {
            auto   task = std::move(m_waiting_task.front());
            HANDLE   finish_event = task->Execute();
            m_waiting_task.pop_front();

            m_running_finish_event.push_back(finish_event);
            m_running_task.push_back(std::move(task));
        }
    }

    void WaitFinishEvent(bool wait_all)
    {
        if (m_running_task.empty())
            return;

        DWORD   index = ::WaitForMultipleObjects((DWORD)m_running_finish_event.size(), m_running_finish_event.data(), wait_all, INFINITE) - WAIT_OBJECT_0;
        if (index < m_running_task.size())
        {
            if (wait_all)
                return;
            else
            {
                m_finish++;
                m_running_task.erase(m_running_task.begin() + index);
                m_running_finish_event.erase(m_running_finish_event.begin() + index);
                return;
            }
        }
        assert(false);
    }
};
/// @endcond
_PHOXO_NAMESPACE_END
