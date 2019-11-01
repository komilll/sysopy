<b>Czas wykonania:</b> Pierwsze zadanie - jeśli mamy już zrobione przeszukiwanie w głąb to całość sprowadza się do zrobienia <b>fork()</b>, wykonywania dalej w dziecku i czekaniu w rodzicu na zakończenie. Max 5min, jeśli jesteśmy odpowiednio przygotowani. Ok. 1h z przygotowaniem systemu do przeszukiwania w głąb

<b>Największa trudność:</b> 

https://github.com/kaszperro/sysopy/blob/master/cw03/zad1/main.c - Krótkie i przejrzyste rozwiązanie do zadania 1 (u siebie zostawiłem logi z cw02/zad2).

<b>Poziom:</b> -

<h1>Tworzenie procesów. Środowisko procesu, sterowanie procesami.</h1>

<h3>Zadanie 1. (40%)</h3>

Zmodyfikuj zadanie 2 z poprzedniego zestawu w taki sposób, iż przeszukiwanie w każdym z odnalezionych (pod)katalogow powinno odbywać sie w osobnym procesie.

<h3>Zadanie 2. Prosty interpreter zadań wsadowych (60%)</h3>

W ramach ćwiczenia należy napisać prosty interpreter zadań wsadowych. Interpreter przyjmuje w argumencie nazwę pliku zawierającego zadanie wsadowe i wykonuje wszystkie polecenia z tego pliku. Polecenia w pliku wsadowym maja postac: 


NazwaProgramu arg1 arg2 ...

co oznacza, że należy wykonać program o nazwie NazwaProgramu z argumentami: arg1, arg2, ...
Na przykład, linia postaci:
ls -l
powinna spowodować wykonanie programu ls z argumentem -l. Lista argumentów może być pusta - wówczas program wykonywany jest bez argumentów. W zadaniu można również przyjąć górne ograniczenie na liczbę argumentów.

Interpreter musi wykonywać polecenia w osobnych procesach. W tym celu, po odczytaniu polecenia do wykonania interpreter tworzy nowy proces potomny. Proces potomny natychmiast wykonuje odpowiednią funkcję z rodziny exec, która spowoduje uruchomienie wskazanego programu z odpowiednimi argumentami. Uwaga: proces potomny powinien uwzględniać zawartość zmiennej środowiskowej PATH - polecenie do wykonania nie musi obejmować ścieżki do uruchamianego programu, jeśli program ten znajduje się w katalogu wymienionym w zmiennej PATH.
Po stworzeniu procesu potomnego, proces interpretera czeka na jego zakończenie i odczytuje status zakończenia. Jeśli proces zakończył się ze statusem 0 interpreter przystępuje do wykonania kolejnej linii pliku wsadowego. W przeciwnym wypadku interpreter wyświetla komunikat o błędzie i kończy pracę. Komunikat ten powinien wskazywać, które polecenie z pliku wsadowego zakończyło się błędem. Zakładamy, że polecenia z pliku wsadowego nie oczekują na żadne wejście z klawiatury. Mogą natomiast wypisywać wyjście na ekran.
