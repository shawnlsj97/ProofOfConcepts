// https://go.dev/blog/pipelines

// Fan-out: allow multiple functions to read from the same channel until it is closed
// Used to distribute work amongst a group of workers to parallelize CPU use and I/O
// Fan-in: allow function to read from multiple inputs and proceed until all are closed by multiplexing input channels onto a single channel that is closed when all the inputs are closed

package main

import (
	"fmt"
	"runtime"
	"sync"
)

func createJobs(jobs chan<- int, numJobs int) {
	for i := 0; i < numJobs; i++ {
		jobs <- i
	}
	close(jobs)
}

func worker(id int, jobs <-chan int) <-chan int {
	out := make(chan int, 1000) // make channel buffered to improve throughput (otherwise will block until read by fanIn)
	go func() {
		fmt.Println("Worker", id, "spawned")
		for j := range jobs {
			out <- j
		}
		close(out)
	}()
	return out
}

// Return slice of read-only channels
func fanOut(jobs <-chan int, numJobs int) []<-chan int {
	var channels []<-chan int
	var numGoroutines = runtime.GOMAXPROCS(0) // Defaults to value of runtime.NumCPU() when arg = 0

	// Limit the number of workers to avoid spawning unnecessary goroutines
	if numJobs < numGoroutines {
		numGoroutines = numJobs
	}

	for i := 0; i < numGoroutines; i++ {
		channels = append(channels, worker(i, jobs))
	}
	return channels
}

// Results read from multiple channels into a single channel
func fanIn(channels []<-chan int, numJobs int) <-chan int {
	var wg sync.WaitGroup
	wg.Add(len(channels))
	out := make(chan int, numJobs) // Buffer output channel to prevent blocking
	for _, c := range channels {
		go func(ch <-chan int) {
			defer wg.Done() // Ensure wg.Done() is called even if there is a panic or early return
			for v := range ch {
				out <- v
			}
		}(c)
	}

	// Launch goroutine to wait for all jobs to be done
	go func() {
		wg.Wait()
		close(out)
	}()
	return out
}

func main() {
	numJobs := 1000
	jobs := make(chan int, numJobs)
	createJobs(jobs, numJobs)

	// FAN OUT - Multiple worker go routines read from same channel until it is closed
	channels := fanOut(jobs, numJobs)

	// FAN IN - Multiplex multiple channels
	result := fanIn(channels, numJobs)
	for v := range result {
		fmt.Println(v)
	}
}
