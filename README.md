## How I approached this problem

First, I read up on external sorting. Although I have heard of the concept, I have never implemented such an
algorithm. Handily I found the topic was well-covered in Knuth's "The Art of Computer Programming", which I have
a copy of. After reviewing this I came up with a basic plan of attack:

1. Implement a naive external sort that achieves optimal asymptotic performance.
2. Benchmark it.
3. Improve. Repeat as desired.

The naive implementation should be straightforward. First of all, I assume for simplicity that there is one disk
drive -- a search quickly turns up a wealth of interesting literature on external sorting when multiple disks are
available. This is not an efficient way to spend a weekend, given that I must first come to grips with external
sorting in general *and* seastar before I can consider the particulars of external sorting with multiple drives.

Optimal performance for a data set of size N is still O(N log N). Given a computer with main memory that can hold 
M records, one can split up the data it R = ceiling(N/M) runs of size M. Sorting each run will take O(M log M) using
an "internal" sorting algorithm. Thus in total this first step of creating runs takes O(R M log M) = O(N log M) time.
Then, we must merge each of the runs. A naive way to do this is to push one element from each run onto a priority
queue. Then, pop the element noting which run, r, it came from and write it to its final location on disk. Then push
another element from the run r onto the queue (if r is not exhausted). Assuming the priority queue is implemented
using a heap, then pushing and popping both take O(log R) time since there is at most 1 element from each run in the
queue. We'll push/pop N times of course, so that the total time to merge comes out to be O(N log R) = O(N log N) as 
we claimed.
(Also the time to create the heap is O(R) == O(N/M) == O(N) if we consider M constant, but this is asymptotically
dominated by the actual merging, as is the original time to create the runs which is also O(N).)

In practice we will probably not be able to simply allocate all available memory for holding records.
The virtual address space for our process may be fragmented so that (for example) allocating 3GiB will not be
possible. Thus the size of each run will likely be much less than the total available memory. This presents an
opportunity for parallelization -- several small runs can be read & sorted at once. M will have to be suitably chosen
by benchmarking and some experimentation.

## Actually doing it

Here are my stream-of-consciouscness notes while attempting this:

The instructions for building seastar are minimal but straightforward. It took me a little bit of time to get it all
set up. However linking against libseastar is turning out to be less straightforward. For one, there are many other
unlisted libraries to link against -- it's a bit of a pain to figure them out by trial and error.
Ideally there would be instructions for *building* the seastar hello world examples in addition to just writing them.

Actually I just discovered a sneaky line in the tutorial about building using pkg-config to determine dependencies.
Kudos, guys!

They don't seem keen on using namespaces in seastar... why is that? For instance `sstring` is not in a namespace.

You guys will probably hate me for finding out that I'm using Windows. But it seems that seastar does not play nicely
with the windows filesystem -- open_file_dma raises an exception when I try to open a file on my Windows host from
my Ubuntu VM.

Seems that a `file` must be wrapped for lambda capture otherwise it becomes a `const file` and member template 
function deduction fails! I took a look at the test cases and got this idea from `fileiotest.cc`.

Actually it seems that in general lambdas qualify their captured values with `const`, since by default their
`operator()` is `const`. In order to circumvent this and modify captured values, one adds the `mutable` keyword to a
lambda declaration.

I now have two problems. For one, it seems that my `main` function is returning right away despite my best efforts.
I have tried waiting on a semaphore or `get`ing the value of a future -- these do not seem to block. But if I do wait
by some other means (like `std::this_tread::sleep_for`) then I never actually *read* from the file using the 
`file::read_dma` method. In fact, the test case where I got this example from fails!