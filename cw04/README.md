<b>Czas wykonania:</b> ok. 8-10h

<b>Największa trudność:</b> Okazuje się, że są problemy z obsługą zadania nr 1 wyłącznie z konsoli. Potrzebna jest druga konsola zarówno na Ubuntu, jak i na Fedorze. Wynika to z faktu, że poprzez używanie Ctrl+Z, kolejne wywołania z jednej konsoli są ignorowane.

Innym problemem jest zadanie 3, gdzie trzeba zwrócić uwagę na poprawną obsługę pipe'ów. W moim wypadku najlepiej zadziałało używanie dwóch osobnych tablic.

<b>Polecane repo:</b>
<ul>
<li>https://github.com/kaszperro/sysopy/tree/master/cw04 - zad 1, w zasadzie bardzo proste. Przejrzałem kilka repo i wszystkie mają taką samą implementację
<li>https://github.com/ixf/sysopy/blob/master/SofinskiBartosz/cw04/zad3/main.c - zad 2, jednak ja zrobiłem to krócej i przejrzyściej
<li>https://github.com/pikinier20/sysopy/blob/master/zestaw5/zad1/main.c - zad 3, nawet nie wiem czy da się napisać krócej niż to. ŚWIETNE!
</ul>

<b>Poziom:</b> raczej fajne, ale tak minimalnie lepiej niż "neutralne"

![batman.gif](batman.gif)

<h1>Zestaw 4. Sygnały i Potoki</h1>

<h3>Zadanie 1 (15%)</h3>

<ul>
<li>Napisz program wypisujący w pętli nieskończonej aktualną godzinę Po odebraniu sygnału SIGTSTP (CTRL+Z) program zatrzymuje się, wypisując komunikat "Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu". Po ponownym wysłaniu SIGTSTP program powraca do pierwotnego wypisywania.
Program powinien również obsługiwać sygnał SIGINT. Po jego odebraniu program wypisuje komunikat "Odebrano sygnał SIGINT" i kończy działanie. W kodzie programu, do przechwycenia sygnałów użyj zarówno funkcji signal, jak i sigaction (np. SIGINT odbierz za pomocą signal, a SIGTSTP za pomocą sigaction).
<li>Zrealizuj powyższe zadanie, tworząc program potomny, który będzie wywoływał jedną z funkcji z rodziny exec skrypt shellowy zawierający zapętlone systemowe polecenie date. Proces macierzysty będzie przychwytywał powyższe sygnały i przekazywał je do procesu potomnego, tj po otrzymaniu SIGTSTP kończy proces potomka, a jeśli ten został wcześniej zakończony, tworzy nowy, wznawiając działanie skryptu, a po otrzymaniu SIGINT kończy działanie potomka (jeśli ten jeszcze pracuje) oraz programu.
</ul>

<h3>Zadanie 2 (35%)</h3>

Napisz program który tworzy proces potomny i wysyła do niego L sygnałów SIGUSR1, a następnie sygnał zakończenia wysyłania SIGUSR2. Potomek po otrzymaniu sygnałów SIGUSR1 od rodzica zaczyna je odsyłać do procesu macierzystego, a po otrzymaniu SIGUSR2 kończy pracę.

Proces macierzysty w zależności od argumentu Type (1,2,3) programu wysyła sygnały na trzy różne sposoby:

<ul>
<li>SIGUSR1, SIGUSR2 za pomocą funkcji kill (15%)
<li>SIGUSR1, SIGUSR2 za pomocą funkcji kill, z tym, że proces macierzysty wysyła kolejny sygnał dopiero po otrzymaniu potwierdzenia odebrania poprzedniego (15%)
<li>wybrane 2 sygnały czasu rzeczywistego za pomocą kill (10%)
</ul>
Program powinien wypisywać informacje o:

<ul>
<li>liczbie wysłanych sygnałów do potomka
<li>liczbie odebranych sygnałów przez potomka
<li>liczbie odebranych sygnałów od potomka
</ul>

Program kończy działanie po zakończeniu pracy potomka albo po otrzymaniu sygnału SIGINT (w tym wypadku od razu wysyła do potomka sygnał SIGUSR2, aby ten zakończył pracę. Wszystkie pozostałe sygnały są blokowane w procesie potomnym).

L i Type są argumentami programu.

<h3>Zadanie 3 (50%)</h3>

Należy rozszerzyć interpreter poleceń z zadania 2 w zestawie 3 (Procesy) tak, by obsługiwał operator pipe - "|". Interpreter czyta kolejne linie z podanego pliku, każda linia ma format

prog1 arg1 ... argn1 | prog2 arg1 ... argn2 | ... | progN arg1 ... argnN

Dla takiej linii interpreter powinien uruchomić wszystkie N poleceń w osobnych procesach, zapewniając przy użyciu potoków nienazwanych oraz funkcji dup2, by wyjście standardowe procesu k było przekierowane do wejścia standardowego procesu (k+1). Można założyć ograniczenie górne na ilość obsługiwanych argumentów oraz ilość połączonych komend w pojedynczym poleceniu (co najmniej 5). Po uruchomieniu ciągu programów składających się na pojedczyne polecenie (linijkę) interpreter powinien oczekiwać na zakończenie wszystkich tych programów.

<b>Uwaga:</b> należy użyć pipe/fork/exec, nie popen
