<b>Czas wykonania:</b> ok. 9-12h

<b>Największa trudność:</b> Odpowiednie zdefiniowanie maszyny stanów. Osobiście rozrysowałem sobie podstawową postać na kartce, jednak obecnie z niej zrezygnowałem.

Techniczna trudność - używanie flagi <b>O_RDWR</b> przy shm_open POSIX. W innym wypadków przy tworzeniu semafora zostanie zwrócony błąd.

<b>Polecane repo:</b> https://github.com/osdnk/sysopy/tree/master/zestaw7 - nie przyglądałem się dokładnie implementacji. Widzę tylko, że jest ładnie podzielone na funkcje. U siebie postawiłem na wrzucenie wszystkiego do głównego "while", w celu widocznie i po prostu ze względu na wielkość zadania. Zasada naszego rozumowania jest podobna, jednak stany różnią się działaniem. U mnie klient jest albo na krześla, albo w kolejce. Nie posiada dodatkowych stanów - cały flow obsługuje stan barbera. Nie korzystałem praktycznie z githuba, poza błędami (np. O_RDWR), więc nic nie mogę polecić konkretnego.

<b>Poziom:</b> zmęczony, po prostu

Zadanie samo w sobie nie jest trudne i skupia się tak naprawdę na obsłudze maszyny stanów. Sama implementacja najlepiej, gdy jest opakowana w funkcje. Wówczas zrobienie punktu drugiego jest bardzo łatwe. Zadanie jest ciekawe, gdy na początku próbuje się znaleźć rozwiązanie, jednak potem synchronizacja wszystkiego, poprawa drobnych błędów jest głównie męcząca. Zadanie nie jest frustrujące - po prostu się zmęczycie przy implementacji, która nie jest pod koniec super ciekawa.

![tired_shiro.gif](tired_shiro.gif)

<h1>IPC - pamieć wspólna, semafory</h1>

<b>Przydatne funkcje:</b>

System V:

<sys/shm.h> <sys/ipc.h> - shmget, shmclt, shmat, shmdt

POSIX:

<sys/mman.h> - shm_open, shm_close, shm_unlink, mmap, munmap

<h3>Problem synchronizacyjny śpiącego golibrody z kolejką</h3>W ramach ćwiczenia 7 należy zaimplementować prawidłowe rozwiązanie problemu śpiącego golibrody z kolejką. Golibroda prowadzi zakład fryzjerski z jednym krzesłem w gabinecie (na którym strzyże klientów) i N krzesłami w poczekalni. Zakład działa według następującego schematu:

<ul>
    <li><b>Golibroda</b></li>
      <ol>
        <li>Golibroda sprawdza, czy w poczekalni oczekują klienci.</li>
        <li>Jeśli w poczekalni nie ma klientów, golibroda zasypia. W przeciwnym razie golibroda zaprasza do strzyżenia klienta, który czekał najdłużej.</li>
        <li>Gdy golibroda skończy strzyżenie, klient opuszcza zakład. Golibroda ponownie sprawdza poczekalnię.</li>
      </ol>
    <li><b>Klient</b></li>
      <ol>
        <li>Po przyjściu do zakładu klient sprawdza co robi golibroda.</li>
        <li>Jeśli golibroda śpi, klient budzi go, siada na krześle w gabinecie i jest strzyżony.</li>
        <li>Jeśli golibroda strzyże innego klienta, nowy klient sprawdza, czy w poczekalni jest wolne krzesło. Jeśli tak, to klient siada w poczekalni. W przeciwnym razie klient opuszcza zakład.</li>
      </ol>
</ul>

Program golibrody wypisuje na ekranie komunikaty o następujących zdarzeniach:
<ol>
    <li>zaśnięcie golibrody,
    <li>obudzenie golibrody,
    <li>zaproszenie klienta z poczekalni do strzyżenia (w komunikacie tym podawany identyfikator strzyżonego klienta),
    <li>rozpoczęcie strzyżenia (z identyfikatorem strzyżonego klienta),
    <li>zakończenie strzyżenia (z identyfikatorem strzyżonego klienta).
</ol>
Każdy klient wypisuje na ekranie komunikaty o następujących zdarzeniach:
<ol>
    <li>obudzenie golibrody,
    <li>zajęcie miejsca na krześle do strzyżenia,
    <li>opuszczenie zakładu po zakończeniu strzyżenia,
    <li>zajęcie miejsca w poczekalni,
    <li>opuszczenie zakładu z powodu braku wolnych miejsc w poczekalni.
</ol>
Każdy komunikat golibrody lub klienta powinien zawierać znacznik czasowy z dokładnością do mikrosekund (można tu wykorzystać funkcję clock_gettime z flagą CLOCK_MONOTONIC). Każdy komunikat klienta powinien ponadto zawierać jego identyfikator.

Zakładamy, że golibroda oraz każdy klient to osobne procesy. Klienci są tworzeni przez jeden proces macierzysty (funkcją fork). Identyfikatorem klienta jest jego PID. Proces tworzący klientów przyjmuje dwa argumenty: liczbę klientów do stworzenia oraz liczbę strzyżeń S. Każdy klient odwiedza w pętli zakład fryzjerski. Gdy klient zostanie ostrzyżony S razy, proces klienta kończy pracę. Proces macierzysty kończy pracę po zakończeniu wszystkich procesów klientów (można wówczas uruchomić w konsoli kolejną partię klientów). Proces golibrody pracuje do momentu otrzymania sygnału SIGTERM. Liczba krzeseł w poczekalni jest podawana w argumencie wywołania golibrody. 

Synchronizacja procesów musi wykluczać zakleszczenia i gwarantować sekwencję zdarzeń zgodną ze schematem działania zakładu. Niedopuszczalne jest na przykład:
<ul>
    <li>budzenie golibrody, który nie śpi,
    <li>zasypianie golibrody gdy w poczekalni czekają klienci,
    <li>zajmowanie miejsca w poczekalni po obudzeniu golibrody (klient powinien od razu siąść na krześle do strzyżenia),
    <li>rozpoczęcie strzyżenia klienta zanim klient zajmie miejsce na krześle do strzyżenia,
    <li>opuszczenie zakładu (przez strzyżonego klienta) przed zakończeniem strzyżenia,
    <li>itd.
 </ul>

Niedopuszczalne jest również zasypianie procesów klientów lub golibrody funkcjami z rodziny sleep.

Szeregowanie klientów do strzyżenia należy zaimplementować w postaci kolejki FIFO w pamięci wspólnej. W pamięci wspólnej można również przechowywać inne niezbędne zmienne (np. zmienną zliczającą klientów oczekujących w poczekalni, flagę wskazującą czy golibroda śpi, etc.). Odpowiednie zasoby do synchronizacji i komunikacji powinny być tworzone przez proces golibrody. Proces ten jest również odpowiedzialny za usunięcie tych zasobów z systemu (przed zakończeniem pracy).

<h2>Zadanie 1</h2>

Zaimplementuj opisany powyżej problem synchronizacyjny wykorzystując semafory i pamięć wspólną z IPC Systemu V.

<h2>Zadanie 2</h2>

Zaimplementuj opisany powyżej problem synchronizacyjny wykorzystując semafory i pamięć wspólną z IPC POSIX.
