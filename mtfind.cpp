#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <thread>


// Check for the presence of a word in a string according to a mask
inline std::tuple<bool, size_t, std::string> find_word_with_mask(const std::string& str, const std::string& mask) {
    size_t str_length = str.length();
    size_t mask_length = mask.length();

    for (size_t i = 0; i <= str_length - mask_length; ++i) {
        bool is_match = true;

        for (size_t j = 0; j < mask_length; ++j) {
            if (mask[j] != '?' && mask[j] != str[i + j]) {
                is_match = false;
                break; // Exit from the inner loop on mismatch
            }
        }

        if (is_match) {
            return std::make_tuple(true, i, str.substr(i, mask_length));
        }
    }

    return std::make_tuple(false, 0, "");
}

class ThreadSafeQueue {
public:
    void enqueue(std::pair<std::vector<std::string>*, size_t> vec) {
        std::unique_lock<std::mutex> lock(mtx_);
        queue_.push(vec);
        cond_var_.notify_one(); // Notify one waiting thread
    }

    std::pair<std::vector<std::string>*, size_t> dequeue() {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_var_.wait(lock, [&] { return !queue_.empty(); }); // Wait if the queue is empty
        auto vec = queue_.front();
        queue_.pop();
        return vec;
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mtx_);
        return queue_.empty();
    }

private:
    std::queue<std::pair<std::vector<std::string>*, size_t>> queue_; // Queue with pointers to vectors of strings and line number
    mutable std::mutex mtx_;
    std::condition_variable cond_var_;
};

// Structure for storing search results
struct SearchResult {
    size_t line_number;
    std::string word;
    size_t position;
};

// Function for reading from a file
void file_reader(const std::string& filename, ThreadSafeQueue& queue, size_t chunk_size) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filename << std::endl;
        return;
    }

    std::string line;
    auto lines = new std::vector<std::string>();
    lines->reserve(chunk_size);

    size_t line_number = 0;

    while (std::getline(file, line)) {
        lines->push_back(line);
        line_number++; // from 1
        if (lines->size() >= chunk_size) {
            queue.enqueue({ lines, line_number - lines->size() }); // Pass the number of the first line in the current vector
            lines = new std::vector<std::string>(); // Create a new vector
            lines->reserve(chunk_size);
        }
    }

    if (!lines->empty()) { // Remaining lines
        queue.enqueue({ lines, line_number - lines->size() });
    }

    queue.enqueue({ nullptr, 0 }); // Notify that reading is complete
}

// Function for processing lines in threads
void worker(ThreadSafeQueue& queue, const std::string& mask, std::vector<SearchResult*>& results, std::mutex& results_mutex) {
    while (true) {
        auto [lines, start_line] = queue.dequeue();
        if (lines == nullptr) break; // End the thread if a nullptr is received

        std::vector<SearchResult*> local_results; // Local vector for storing results

        for (size_t i = 0; i < lines->size(); ++i) {
            auto [found, position, word] = find_word_with_mask((*lines)[i], mask);
            if (found) {
                local_results.push_back(new SearchResult{ start_line + i + 1, word, position });
            }
        }

        // Move local results to the shared vector under mutex protection
        {
            std::lock_guard<std::mutex> lock(results_mutex);
            results.insert(results.end(), local_results.begin(), local_results.end());
        }

        delete lines; // Delete the vector after processing
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <filename> <mask>" << std::endl;
        return 1;
    }

    std::string filename = argv[1]; //"input.txt"; // 
    std::string mask = argv[2]; // "?a?d"; // 

    const size_t chunk_size = 10; // Number of lines to read at a time
    const size_t num_workers = std::thread::hardware_concurrency() - 1; // Number of threads //TODO: !!!
    ThreadSafeQueue queue;

    // Vector for storing all results
    std::vector<SearchResult*> results;
    std::mutex results_mutex; // Mutex for protecting access to results

    auto start_time = std::chrono::high_resolution_clock::now();

    // Create and launch the worker pool
    std::vector<std::future<void>> workers;
    for (size_t i = 0; i < num_workers; ++i) {
        workers.push_back(std::async(std::launch::async, worker, std::ref(queue), mask, std::ref(results), std::ref(results_mutex)));
    }

    // Start the file reading thread
    std::future<void> file_thread = std::async(std::launch::async, file_reader, filename, std::ref(queue), chunk_size);

    // Wait for the file reading thread to finish
    file_thread.get();

    // After reading the file, notify the workers to stop
    for (size_t i = 0; i < num_workers; ++i) {
        queue.enqueue({ nullptr, 0 }); // Notify workers of the completion
    }

    // Wait for all workers to finish
    for (auto& worker : workers) {
        worker.get(); // Wait for each worker to finish
    }

    // Sort results by line number
    std::sort(results.begin(), results.end(), [](const SearchResult* a, const SearchResult* b) {
        return a->line_number < b->line_number;
        });

    // Output all results in the main thread
    std::cout << results.size() << std::endl;
    for (const auto& result : results) {
        std::cout << result->line_number << " " << result->position + 1 << " " << result->word << std::endl;
        delete result;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds." << std::endl;

    return 0;
}

