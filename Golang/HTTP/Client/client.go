package main

import (
    "fmt"
    "net/http"
    "sync"
    "time"
)

const (
    totalRequests   = 1000000
    concurrentLimit = 8 // Limit active goroutines to 1000 at any time
)

var (
    semaphore   = make(chan struct{}, concurrentLimit)
    wg          sync.WaitGroup
    httpClient  *http.Client
)

func makeRequest(url string) {
    defer wg.Done()
    resp, err := httpClient.Get(url) // Using the shared httpClient
    if err != nil {
        fmt.Println("Request failed:", err)
        return
    }
    defer resp.Body.Close()
    fmt.Println("Response Status:", resp.Status)
}

func main() {
    // Create an HTTP client with custom transport settings for connection pooling
    httpClient = &http.Client{
        Transport: &http.Transport{
            MaxIdleConns:        1000,
            MaxIdleConnsPerHost: 100,
            IdleConnTimeout:     30 * time.Second,
        },
        Timeout: 10 * time.Second, // Set a global request timeout
    }

    start := time.Now()
    url := "http://localhost:8080"

    // Start sending requests
    for i := 0; i < totalRequests; i++ {
        wg.Add(1)
        semaphore <- struct{}{} // Acquiring semaphore
        go func() {
            makeRequest(url)
            <-semaphore // Releasing semaphore
        }()
    }

    wg.Wait() // Wait for all requests to complete
    elapsed := time.Since(start)
    fmt.Printf("Completed %d requests in %s\n", totalRequests, elapsed)
}
