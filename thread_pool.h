#ifndef thread_pool_
#define thread_pool_
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <vector>
#include <atomic>
#include <type_traits>
#include <iostream>
#include <stdexcept>

using function_t = std::function<void()>;

// Klasa tast_queue
class task_queue
{

private:
  // Queue taskova koji mogu doći sa različitih niti
  std::queue<function_t> tasks_;

  // Mutex i condiditon variabla za sihronizaciju
  std::mutex mtx_;
  std::condition_variable cv_;
  bool stopped_ = false;

public:
  // Ukoliko je queue prazan, niti će blokirati
  // ukoliko ne onda izvrsavamo datu nit
  bool blocking_pop(function_t &fun)
  {
    std::unique_lock<std::mutex> queue_lock{mtx_};
    while (tasks_.empty())
    {
      if (stopped_)
        return false;
      cv_.wait(queue_lock);
    }
    fun = std::move(tasks_.front());
    tasks_.pop();
    return true;
  }

  // Funckija za dodavanje novog taska
  template <typename F>
  void push(F &&f)
  {
    {
      std::unique_lock<std::mutex> queue_lock{mtx_};
      tasks_.emplace(std::forward<F>(f));
    }
    cv_.notify_one();
  }

  // Funckija za signalizizaciju  da se queue zaustavi
  void stop()
  {
    {
      std::unique_lock<std::mutex> queue_lock{mtx_};
      stopped_ = true;
    }
    cv_.notify_all();
  }
};

// Klasa thread_pool
class thread_pool
{

private:
  void run(int i)
  {
    while (true)
    {
      function_t fun;
      int curent_task = i;
      bool stolen_task = false;
      
      do
      {
        if (tasks_.at(curent_task)->blocking_pop(fun))
        {
          stolen_task = true;
          break;
        }
        ++curent_task;
        if (curent_task == thread_number_)
          curent_task = 0;
      } while (curent_task != i);

      if (!stolen_task && !(*tasks_[i]).blocking_pop(fun))
        return;
      fun();
    }
  }

  // Oznacava broj threadova
  const size_t thread_number_;

  // Predstavlja vektor threadova koji ce preuzimati iz
  // queue-a sve dok queue nije prazan
  std::vector<std::thread> threads_;

  // U datoj implementaciji, svi zadaci, koji mogu doći sa različitih
  // niti, se smještaju u jedan queue (opisan klasom task_queue). Taj queue
  // je dijeljen izmedu svih niti u thread pool-u.
  // task_queue tasks_;

  std::vector<task_queue *> tasks_;
  int taskNumber = 0;

public:
  // Konstruktor za thread_pool
  // Prilikom inicijalizacije thread pool-a specificira se broj niti.
  // Ukoliko se ne specificira, koristi se onoliko niti koliko je hardverski
  // podržano na datoj platformi.
  thread_pool(size_t th_num = std::thread::hardware_concurrency()) : thread_number_{th_num}
  {
    // Kreiramo thread te ga dodajemo u niz threadova
    for (int i = 0; i < thread_number_; ++i)
    {
      threads_.emplace_back(std::thread{[this, i]() { run(i); }});
      tasks_.emplace_back(new task_queue());
    }
  }

  // Destruktor za thread pool
  ~thread_pool()
  {
    // Signaliziramo da se queue zaustavi
    // tasks_.stop();
    for (auto i = 0; i < thread_number_; ++i)
      (*tasks_[i]).stop();

    // Prije samog zaustavljanja svi zadaci se moraju završiti.
    for (auto &t : threads_)
      t.join();
  }

  // Funkcija koja nam dodaje u tasks queue novi zadatak
  template <typename T>
  void async(T &&fun)
  {
    //tasks_.push(std::forward<T>(fun));
    int position = taskNumber++ % thread_number_;
    (*tasks_[position]).push(std::forward<T>(fun));
  }
};

#endif /* ifndef thread_pool_ */