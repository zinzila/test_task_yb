
# Developemnt

## Task analysis

Assumptions about task:
- provided API SHALL NOT change
  - provided implementations SHALL NOT change
- provided API can be extended

### Object allocation

From textual description the task seems to be clear, but provided code stubs create confusion on what behavior is needed.
Provided API has some ambiguities about its behavior.
From the provided code ownership model for the `IEvent` instances is unclear. The allocation point is defined, but the point
for releasing the object is not. When allocating, from the provided stub the following seems to be the case:
- a call to an undefined method `ReserveEvent()` from `IEventProcessor::Reserve` does not take any arguments or a type
- this means, that an implementation of `ReserveEvent()` cannot do allocation as the size is not known
- at the same time, the return value (supposedly a `pair`) shall contain a pointer where an object is supposed to be
  allocated, e.g. via placement new.

Similar reasoning is applicable for bulk allocation method `ReserveRange`, but the situation is even more cumbersome, as the
desired size for a list of events is nowhere in the API at the point where reservation is supposed to be made.

### Execution

Assume, that execution is scheduled with the call to `IEventProcessor::Commit`.
The order of execution is a bit unclear, as there is a `sequence_number_` which one can assume to be the implied order of event
execution, however this would complicate execution process quite a lot. Also from the client point of view of this API it would
be close to impossible to control the order, as this sequence number is created internally and only available for the client
for reading. Therefore assume that the order of the execution is defined by the order of the calls to `IEventProcessor::Commit`

### Event lifetime

The API does not provide clear point where an event shall be deallocated. It is also not clear, whether an event can be executed
more than once, e.g. one can call `IEventProcessor::Commit` with the same sequence number multiple times to trigger execution.
Other possible behavior would be to invalidate the event after `IEvent::Process` has being completed. This would imply that
`ReservedEvent` object is in undefined state after it was executed, which seems to be an unsafe choice for the API. Hence, assume
that once reserved the event can be triggered multiple times.
To be able to deallocate an additional method `IEventProcessor::Release` would have to be introduced.

### Threading

Assume, that reserving and committing of the same event happens from the same thread, as hinted by the task description. Also
assume no thread safety for the `IEvent::Process` method - shall be handled by the client if needed.

## Solution approach

For the solution assume that execution speed is the top goal and resource usage is of a least importance. This means that excessive
RAM usage to achieve higher execution speed is accepted.
To speed up execution memory allocations should be minimized, thus it makes sense to preallocate as much as possible, especially given
that the provided code hints to the usage of placement `new`.
Preallocating memory for the objects implies some kind of memory management. To simplify this process and also be able to reserve space
for an event without knowing the actual size of the event, it is possible to set the maximum allowed event size. This way it will be
possible to give back reserved buffers for usage with placement new without knowing the actual size and since all the sizes are the\
same to avoid complex problems of handling fragmentation. Major disadvantage is excessive memory usage, which is assumed acceptable for
this task.

## Simple runs

Run app multiple times to collect some timings. Results are in CSV format:
```
producer_count, event_per_producer, total_time, average_per_producer
```

Command to collect:

```
for i in {0..20}; do nice -20 ./build/youbourse ; sleep 2; done \
    | tee "results/multipleruns-($(git describe --tags --dirty)).csv"
```

## Benchmarking

For benchmarking Catch2 benchmark capability is used. Following command used to collect results:

```
./build/tests/benchmark | tee "results/benchmark-($(git describe --tags --dirty)).txt"
```

Source code for benchmark: `tests/benchmark.cpp`

## Profiling

Using `valgrind` by means of `callgrind` tool.

```
valgrind --tool=callgrind --callgrind-out-file="results/callgrind-($(git describe --tags --dirty)).out"  ./build/youbourse
```

The results are analyzed using [kCachegrind](https://kcachegrind.github.io/html/Home.html)
