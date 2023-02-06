/* Creates stackful coroutines from functions.
 * Stackful coroutines are like functions except that they can be
 * suspended mid-execution and later resumed from the point they were
 * suspended.
 *
 * CreateCoroutine(function, ...)
 * Returns a suspended coroutine that will call function when resumed.
 * If provided, arguments must be integers or pointers. The arguments
 * must also match the types in the function declaration but this is not
 * checked.
 *
 * ResumeCoroutine(coroutine)
 * Resumes a suspended coroutine. Returns a new suspended coroutine if
 * function suspends; otherwise returns NULL. The original coroutine is
 * invalid in either case.
 * If a coroutine is executing the behavior is undefined.
 *
 * SuspendCoroutine()
 * Suspends function and returns control to the ResumeCoroutine call
 * that resumed the coroutine.
 * If a coroutine is not executing the behavior is undefined.
 *
 * SUSPEND_UNTIL(cond)
 * Suspends the coroutine until cond is true.
 *
 * SUSPEND_WHILE(cond)
 * Suspends the coroutine while cond is true.
 *
 * CreateCoroutineTask(coroutine)
 * Creates a task that will resume coroutine each frame until it exits
 * without suspending.
 *
 * Coroutines should be run to completion, otherwise any resources they
 * allocate will be leaked (including the coroutine stack itself), and
 * any invariants that they enforce could be violated.
 *
 * The coroutine is run on the caller's stack because pokeemerald does
 * not have enough free IWRAM to allocate a second stack. When resumed/
 * suspended, the stack is copied to the heap. This has two major
 * consequences:
 * 1. If Alloc fails the memory is left in an unknown state. The code
 *    therefore resets the game.
 * 2. Any pointers to local variables are dangling after a context
 *    switch.
 * This implementation is designed for use-cases where there are few
 * resumes/suspends overall per frame, and the resumes of each coroutine
 * occur with the same sp each time.
 *
 * The implementation also assumes that after a Free the freed memory
 * will not be written to before the next Alloc. */

#ifndef GUARD_STACKFUL_COROUTINE_H
#define GUARD_STACKFUL_COROUTINE_H

union CoroutineArgument
{
    s32 as_s32;
    u32 as_u32;
    void *as_ptr;
    const void *as_const_ptr;
} __attribute__((transparent_union));

struct Coroutine;

#define CreateCoroutine(function, ...) CAT(CreateCoroutine, NARG_8(__VA_ARGS__))(function, ##__VA_ARGS__)

struct Coroutine *CreateCoroutine0(void (*)(void));
struct Coroutine *CreateCoroutine1(void (*)(), union CoroutineArgument);
struct Coroutine *ResumeCoroutine(struct Coroutine *);
void SuspendCoroutine(void);

#define SUSPEND_UNTIL(cond) while (!(cond)) SuspendCoroutine()
#define SUSPEND_WHILE(cond) while (cond) SuspendCoroutine()

void CreateCoroutineTask(struct Coroutine *, u8 priority);

#endif
