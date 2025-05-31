package main

import (
    "fmt"
    "log"
    "net/http"
    "runtime"
)

const numWorkers = 4

func worker(id int, jobs <-chan *http.Request, results chan<- *http.Response) {
    for req := range jobs {
        // Process the request here
        fmt.Printf("Worker %d processing request: %v\n", id, req.URL.Path)
        // Placeholder response
        results <- &http.Response{StatusCode: http.StatusOK}
    }
}

func main() {
    runtime.GOMAXPROCS(numWorkers)
    jobs := make(chan *http.Request, numWorkers)
    results := make(chan *http.Response, numWorkers)

    for i := 0; i < numWorkers; i++ {
        go worker(i, jobs, results)
    }

    http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
        jobs <- r
        res := <-results
        w.WriteHeader(res.StatusCode)
    })

    log.Println("Starting server on :8080...")
    log.Fatal(http.ListenAndServe(":8080", nil))
}
