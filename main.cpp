#include "thread_pool.h"
#include <chrono>
#include <algorithm>

std::vector<int> calculated_values;
std::mutex mtx;

// Funckija za dodavanje izracunate vrijednosi u niz
void add_calculated_value(int v)
{
  std::unique_lock<std::mutex> result_lock(mtx);
  calculated_values.push_back(v);
}

// Funkcija za ispis rezultata
void print_calculated_values()
{
  // Sortiramo vrijednosti u nizu zato sto
  // kada smo dodavali vrijednost u niz neke smo izracuanli brze a neke sporije 
  // te postoji mogucnost da vrijednosti nisu sortirane
  std::sort(calculated_values.begin(), calculated_values.end());

  // Vrsimo ispis vrijednosti na ekran
  std::unique_lock<std::mutex> result_lock(mtx);
  for (const auto &el : calculated_values)
    std::cout << el << std::endl;
}

// Funkcija za racunanje fibonacijevog broja
int fib(size_t n)
{
  if (n == 0)
    return 0;
  if (n == 1)
    return 1;
  return fib(n - 1) + fib(n - 2);
}

// Funkcija koju proslijedjujemo u tread pool
int call_fib(size_t n)
{
  // Racunanje fibonacijevog broja 
  // za odgovarajucu vrijednost n
  int result = fib(n);

  // Dodajemo rezultat u niz
  add_calculated_value(result);
  return result;
}

int main(int argc, char *argv[])
{
  // Kreiramo thread pool
  thread_pool tp;

  // Dodajemo elemente u thread pool
  for (int i = 0; i < 40; ++i)
  {
    tp.async(std::bind(call_fib, i));
  }

  // for(int i = 0; i < 40; ++i) {
  // tp.async( [i](){ call_fib(i);} );
  // }

  // Spavamo 5 sekundi te vrsimo ispis vrijednosti
  std::this_thread::sleep_for(std::chrono::seconds{5});
  print_calculated_values();

  return 0;
}