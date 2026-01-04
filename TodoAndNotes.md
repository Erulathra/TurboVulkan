# TODO and Notes

There are some random thoughts and todos that I think I would need to implement in the near future.

## Rendering

### [TODO] Destruction Queue
This class was written just before I started to learn about DOD and also before first vulkan abstraction rewrite. I need
to rewrite whole destruction proces to only store handles. They are cheap to access especially when we sort them in ascending
order. It would simplify destruction process and move some computation overhead to one place. Moreover, it would resolve
problem of double freeing the memory when resource is enqueued to destroy but the handle is still valid.

### [TODO] Split gpu resources to hot and cold data
To better utilize cache it would be great to split resource structures into cold and hot data structures.

### Mapped memory vs staging buffers
I thought that staging buffers would be much faster than mapped memory but according to this and my few benchmarks for
material uniforms mapped memory is much faster. __For assets staging buffer is still used to reduce memory footprint.__

### Scene graph
I thought that using DoD approach to scene graph would result in better performance but the real bottleneck was sorting
entities. In some simple benchmarks cache misses in DFS approach are less costly than sorting all dirty entities.

## Assets management
### Lifetime
Initially I planed to create some ref-counting system to manage assets lifetime, but it would create a few problems with
multithreading or performance. For now, I think that storing assets in memory per level would last.