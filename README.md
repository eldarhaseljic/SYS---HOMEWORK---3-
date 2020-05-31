# SYS---HOMEWORK---3-
Fakultet elektortehnike Tuzla - Sistemsko programiranje

## 1 Problem 1

U fajluthread_pool.h, u prilogu zada ́ce, data je pojednostavljena im-
plementacija thread pool-a. U datoj implementaciji zadaci koji se mogu
izvrˇsavati u thread pool-u predstavljeni sustd::function<void()>tipom.
Zadaci se dodjeljuju thread pool-u pozivom metodaasync. Da bi se u
thread pool mogla poslati funkcija koja prima parametre, mogu ́ce je koris-
titi funckijustd::bind, koja kao parametre primaCallableobjekat (bilo
ˇsta ˇsto podrˇzava operator()), te argumente koji ́ce biti proslijeeni prilikom
poziva navedenog operatora. Alternativno, umjestostd::bind, mogu ́ce je
koristiti lambda izraze za sliˇcnu namjenu. Ovo je prikazano u sljede ́cem
primjeru koriˇstenja thread pool-a. Alternativna varijanta prikazana je u
zakomentiranom dijelu koda.

```
int main(int argc, char *argv[])
{
thread_pool tp;
```
```
for(int i = 0; i < 40; ++i) {
tp.async(std::bind(call_fib, i));
}
```
```
// for(int i = 0; i < 40; ++i) {
// tp.async( [i](){ call_fib(i);} );
// }
```
```
std::this_thread::sleep_for(std::chrono::seconds{5});
print_calculated_values();
```
```
return 0;
}
```
```
Kompletan primjer koriˇstenja thread pool-a dat je u fajlumain.cpp, u pri-
logu zada ́ce. Funkcijeadd_calculated_valueiprint_calculated_values
su dodane da bi se moglo potvrditi da thread pool zaista izvrˇsava zadatke
koji su mu dodijeljeni.
Prilikom inicijalizacije thread pool-a specificira se broj niti. Ukoliko se
ne specificira, koristi se onoliko niti koliko je hardverski podrˇzano na datoj
platformi. U datoj implementaciji, svi zadaci, koji mogu do ́ci sa razliˇcitih
niti, se smjeˇstaju u jedan queue (opisan klasomtask_queue). Taj queue
```

je dijeljen izmeu svih niti u thread pool-u, tako da je neophodno da bude
sinhronizovan, za ˇsto su koriˇstenistd::mutexistd::condition_variable.
Niti iz pool-a ́ce preuzimati zadatke iz queue-a sve dok queue nije prazan.
Ukoliko je queue prazan, niti ́ce blokirati, osim ako nije prethodno signal-
izirano da se queue zaustavi. U sluˇcaju da je signalizirano da se queue
zaustavi, prije samog zaustavljanja svi zadaci se moraju zavrˇsiti. Opisana
implementacija je simboliˇcno prikazana na sljede ́coj slici (1).

```
Slika 1: Inicijalna implementacija
```
Strelica predstavlja zadatak, pravougaonik queue, a krug nit. Duˇzina
strelice simboliˇcno predstavlja duˇzinu zadatka, a razliˇciti smjerovi oznaˇcavaju
da zadaci mogu do ́ci sa razliˇcitih niti.
U zada ́ci je potrebno implementirati sljede ́ce:

1. Modificirati opisanu implementaciju thread pool-a tako da se interno,
    umjesto jednog, koristi po jedan queue za svaku nit. Zadaci koji se
    dodjeljuju thread pool-u rasporeuju se po round-robin principu u
    razliˇcite queue-ove (2). Princip rada sa svakim pojedinaˇcnim queue-
    on je identiˇcan kao i u datoj implementaciji.
2. Dodatno na stavku 1, implementirati funkcionalnost krae zadataka
    (task stealing, 3). Naime, ukoliko neka nit izvrˇsi sve zadatke iz svoga
    queue-a, tada ́ce pokuˇsati “ukrasti” zadatak iz queue-a sljede ́ce niti,
    ukoliko postoji, u suprotnom ́ce nastaviti traˇziti zadatak u queue-u
    svake sljede ́ce niti, sve dok ne pronae zadatak ili ustanovi da nema


```
Slika 2: Implementacija sa viˇse internih queue-ova
```
zadataka ni u jednom queue-u. Ukoliko nit pronae zadatak, preuzet
́ce ga iz queue-a u kojem ga je pronaˇsla, a ukoliko se ispostavi da
nema zadataka koji ˇcekaju na izvrˇsenje, nit ide na spavanje, dok je ne
probudi novi zadatak koji se doda u njen queue ili dok se thread pool
ne zaustavi.

```
Slika 3: Implementacija satask stealing-om
```
