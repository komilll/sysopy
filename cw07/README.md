<b>Czas wykonania:</b> ok. 6-8h

<b>Największa trudność:</b> Raczej dość przystępne w wykonywaniu i debugowaniu ćwiczenie. Osobiście miałem tylko problem, że dla -O3 był ignorowany while, w który wrzuciłem sem_wait/sem_post, aby cały czas coś faktycznie robił.

<b>Polecane repo:</b> https://github.com/osdnk/sysopy/tree/master/zestaw9 - generalnie rozwiązanie problemu nie ma wielu opcji, przynajmniej ja ich nie widzę. Możecie rozbić semafory/mutexy (jak ja) lub używać jednego z odpowiednimi indeksami. Do tego możecie mieć inną kolejność (minimalnie) lockowania semaforów/mutexów, ale generalnie każde rozwiązanie będzie się pokrywać w 90%.

<b>Poziom:</b> całkiem fajne i pouczające

![taiga_smile.gif](taiga_smile.gif)

Ostatni gif dedykuję całości tego przedmiotu (w dużej mierze odnosi się dalej do ćwiczenia). To była piękna przygoda. Cieszę się, jeśli komukolwiek pomogły moje wpisy. Trzymajcie się mordeczki.

<h1>Zadania - Zestaw 7</h1>

<h2>Opis problemu</h2>

Jednym z często spotykanych problemów synchronizacji jest "Problem producentów i konsumentów". Grupa producentów zapisuje dane w określonym miejscu, a konsumenci je pobierają (konsumują). Jedno z rozwiązań opiera się na wykorzystaniu tablicy działającej jak bufor cykliczny, co pozwala na zamortyzowanie chwilowych różnic w szybkości działania producenta i konsumenta oraz niedopuszczenie do nadprodukcji. Tę wersję problemu nazywa się problemem z ograniczonym buforem. Pojedynczy element bufora jest sekcją krytyczną, w której przebywać może tylko jeden producent albo konsument.

<h2>Zadanie</h2>

Zaimplementuj program P+K wątkowy dla P producentów i K konsumentów, działających na tym samym buforze. Rolę bufora pełni globalna tablica N wskaźników do stringów o różnej długości. Producent produkuje i wstawia do bufora porcje różnej wielkości. Konsument pobiera, poszukując porcji określonej wielkości.

Producent działa następująco:
<ul>
    <li>Jeżeli bufor nie jest pełny, to producent czyta kolejną linię z pliku tekstowego zadanego jako parametr programu (powinien mieć kilka tysięcy linii, np całość "Pana Tadeusza"), alokuje pamięć i umieszcza w buforze wskaźnik do tekstu.
    <li>Jeżeli bufor jest pełny, to producent powinien zaczekać do momentu usunięcia wartości z bufora przez konsumenta.
    <li>Każda kolejna wartość jest produkowana w następnym elemencie bufora (producent nie zaczyna od początku tablicy, szukając pierwszego wolnego miejsca, lecz pamięta pozycję, gdzie poprzedni producent wstawił wartość, co zapewnia równomierną produkcję dla całego bufora.
    <li>Po wstawieniu wartości do ostatniego elementu tablicy producent zaczyna cyklicznie wstawiać elementy  od początku tablicy.
</ul>

Konsument działa następująco:
<ul>
    <li>jeżeli bufor jest pusty, to konsument powinien zaczekać do momentu umieszczenia wartości w buforze przez producenta.
    <li>jeżeli bufor nie jest pusty, to konsument pobiera, usuwa wartość z bufora i sprawdza, czy długość pobranego napisu jest w zależności od wartości podanego argumentu równa, większa, bądź mniejsza podanej jako argument wartości L, jeśli tak, to wypisuje nr indeksu tablicy i ten napis.
    <li>praca konsumentów jest analogiczna do pracy producentów (konsument nie zaczyna od początku, szukając pierwszego wyprodukowanego elementu, ale pamięta pozycję, gdzie poprzedni konsument pobrał wartość, co zapewnia równomierną konsumpcję dla całego bufora.
    <li>Po pobraniu wartości ostatniego elementu tablicy konsument zaczyna cyklicznie pobierać elementy  od początku tablicy.
</ul>

Wątki powinny działać w pętli nieskończonej i kończyć się:
<ul>
    <li>jeśli nk>0, po upływie nk sekund,
    <li>jeśli nk=0, po przeczytaniu ostatniego wersu pliku tekstowego lub po odebraniu przez proces główny sygnału CTRL-C.
</ul>

Program powinien umożliwić uruchomienie trybu opisowego (każdy producent i konsument raportuje swoją pracę) oraz uproszczonego (informacje wypisują tylko konsumenci, jeśli odnajdą odpowiedni wynik).

Program powinien wczytać z linii poleceń plik konfiguracyjny, w którym są ustawione parametry P, K, N, nazwa pliku, L, tryb wyszukiwania, tryb wypisywania informacji oraz nk.

Należy wykonać dwie wersje rozwiązania:
<ol>
    <li>Rozwiązanie wykorzystujące do synchronizacji muteks i zmienne warunkowe (zgodne z typem rozwiązań problemu współbieżnego stosującego monitor) (50%)
    <li>Rozwiązanie wykorzystujące do synchronizacji semafory nienazwane standardu POSIX (zgodne z typem rozwiązań problemu współbieżnego stosującego semafory) (50
</ol>
