#include <stdint.h>
#include <stdbool.h>

struct queue_node {
    struct queue_node* next;
};

struct minqueue_node {
    struct queue_node* next;
};

struct queue {
    struct queue_node* head;
    struct queue_node* tail;
    struct minqueue_node dummy;
};

#define CAS(ptr, oldval, newval) __sync_bool_compare_and_swap(ptr, oldval, newval)

// Initialize queue for use
static inline void queue_init(struct queue* queue) {
    queue->dummy.next = (struct queue_node*)queue;
    queue->head = (struct queue_node*)&queue->dummy;
    queue->tail = (struct queue_node*)&queue->dummy;
}

// Internal enqueue function (lock-free)
static void queue_put_node(struct queue* queue, struct queue_node* node) {
    struct queue_node* tail;
    node->next = (struct queue_node*)queue;

    for (;;) {
        tail = queue->tail;
        if (CAS(&tail->next, (struct queue_node*)queue, node)) {
            CAS(&queue->tail, tail, node);
            return;
        }
        CAS(&queue->tail, tail, tail->next);
    }
}

// Public enqueue API
static inline void queue_put(struct queue* queue, struct queue_node* node) {
    queue_put_node(queue, node);
    // User may implement a signal/flag here if needed
}

// Enqueue from IRQ context
static inline void queue_put_from_interrupt(struct queue* queue, struct queue_node* node) {
    queue_put_node(queue, node);
    // Interrupt-driven signal mechanism could be triggered here
}

// Lock-free dequeue operation
static inline struct queue_node* queue_get(struct queue* queue) {
    struct queue_node* head, * tail, * next;

    for (;;) {
        head = queue->head;
        tail = queue->tail;
        next = head->next;

        if (head != queue->head)
            continue;

        if (head == tail) {
            if (next == (struct queue_node*)queue)
                return NULL;
            CAS(&queue->tail, tail, next);
            continue;
        }

        if (CAS(&queue->head, head, next)) {
            if (head != (struct queue_node*)&queue->dummy)
                return head;
            queue_put_node(queue, head);
        }
    }
}
