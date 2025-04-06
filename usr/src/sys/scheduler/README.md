TOS-7 Scheduler Modular Plan
============================

Goal:
Design a clean, modular scheduler with support for SMP and real-time features,
without sacrificing simplicity, determinism, or code clarity.

---------------------------------------------------------------------
1. Core Structure (Completed)
---------------------------------------------------------------------

[x] Per-CPU current_thread[]
[x] Runqueue per CPU, class and priority
[x] Scheduling classes (TS, RT, SYS)
[x] scheduler_enqueue() and scheduler_pick_next()
[x] Per-CPU sleepqueue
[x] scheduler_tick() + preemption logic

Design is clean and modular – already functional for basic RTOS-level multitasking.

---------------------------------------------------------------------
2. Module A – Scheduler Locking
---------------------------------------------------------------------

Status: Not implemented  
Purpose: Prevent preemption during critical sections (e.g., inside context_switch).

Interface:
    uint32_t scheduler_lock(void);
    void     scheduler_unlock(uint32_t key);

Location: scheduler/lock.c

---------------------------------------------------------------------
3. Module B – Timeout & Sleep API
---------------------------------------------------------------------

Status: In progress (scheduler_sleep)  
Purpose: Abstract timeout sleep logic for kernel threads and future POSIX libc use.

Interface:
    void thread_sleep_ms(uint64_t ms);
    bool thread_has_timed_out(thread_t *t);

Location: scheduler/sleep.c

---------------------------------------------------------------------
4. Module C – Cooperative Thread Control
---------------------------------------------------------------------

Status: Not implemented  
Purpose: Enable yield(), suspend(), and resume() for cooperative thread control.

Interface:
    void thread_yield(void);
    void thread_suspend(thread_t *t);
    void thread_resume(thread_t *t);

Location: scheduler/control.c

---------------------------------------------------------------------
5. Module D – Tickless Scheduler Support
---------------------------------------------------------------------

Status: Not implemented  
Purpose: Reduce power usage and interrupt load by skipping unnecessary ticks.

Interface:
    uint64_t scheduler_next_wakeup(cpu_id_t cpu);

Location: scheduler/tickless.c  
Used by: timer.c to set the next physical timer wakeup.

---------------------------------------------------------------------
6. Module E – Real-Time Enhancements
---------------------------------------------------------------------

Status: Not implemented  
Purpose: Introduce deadlines and deterministic behavior for SCHED_RT threads.

Interface:
    void thread_set_deadline(thread_t *t, uint64_t deadline);
    bool thread_has_missed_deadline(thread_t *t);

Location: scheduler/rt.c

---------------------------------------------------------------------
7. Future Modules (To be defined)
---------------------------------------------------------------------

- CPU balancing
- Affinity policies
- Userland scheduling hints
- Tracing & profiling (optional)

---------------------------------------------------------------------
Recommended Folder Structure
---------------------------------------------------------------------

scheduler/
|
├── base.c          // enqueue/pick/tick/timer logic
├── sleep.c         // sleepqueue + timer tick handling
├── rt.c            // deadline support
├── control.c       // yield/suspend/resume logic
├── lock.c          // scheduler_lock/unlock
├── tickless.c      // tickless wakeup logic
├── scheduler.h     // public interface
├── sched_class.h   // TS/RT/SYS class interfaces

---------------------------------------------------------------------
Design Principles
---------------------------------------------------------------------

- One module = one responsibility
- No #ifdef SMP, #ifdef TICKLESS
- Avoid shared state between CPUs (SMP-safe by design)
- Clean ANSI C (C89), no C++-isms
- Inspired by Plan 9 / UNIX V7:
  "Simple things simple, complex things possible"

---------------------------------------------------------------------
Next Steps
---------------------------------------------------------------------

1. Implement scheduler_lock() – simple and minimally invasive
2. Move sleepqueue logic into its own sleep.c module
3. Document thread_t for RT-related fields
4. Add deadline field and RT API, without affecting SYS/TS logic
