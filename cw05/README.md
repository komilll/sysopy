<b>Czas wykonania:</b> ok. 6-8h

<b>Największa trudność:</b> Od razu używajcie dwóch konsol, bo przy tym się wygłupiłem. Poza tym:
<ul>
  <li>MESSAGE_SIZE to sizeof(struct) - sizeof(long), ponieważ <b>mtype</b> nie wlicza się w długość wiadomości (System V)
  <li>mtext musi być na końcu struktury
  <li>należy nadawać uprawnienia do kolejek (zarówno POSIX, jak i System V), ponieważ inaczej będą błędy przy tworzeniu/próbie dostania się do kolejki
  <li>często używałem sscanf lub sprintf do przekazywania id/kluczy kolejek
  <li>strcmp - czytanie linijka po linijce - należy zastąpić ostatni znak '\n', znakiem końca '0'
</ul>

Jeśli zrozumiecie zasadę działania to jest proste, najwięcej czasu schodzi na obsługę subtelnych błędów wymienionych powyżej. Drugie zadanie to kopiuj-wklej pierwszego i zmiana funkcji.

<b>Polecane repo:</b>
<ul>
<li>https://github.com/osdnk/sysopy/tree/master/zestaw6 - osdnk ma najczystszą implementację i widać od razu jaki jest workflow, choć dzienni mieli więcej komend do obsłużenia. Nie zmienia to zupełnie nic, po prostu dodatkowe przekierowanie w switchu
<li>pikinier20 - też spoko
<li>kaszperro - tradycyjnie
</ul>

<b>Poziom:</b> lekkie zdenerwowanie (przez te subtelne problemy, reszta easy)

![slightly_angry.gif](slightly_angry.gif)

<h1>IPC - kolejki komunikatów</h1>

<b>Przydatne funkcje:</b>

System V:

<sys/msg.h> <sys/ipc.h> - msgget, msgctl, msgsnd, msgrcv, ftok

POSIX:

<mqueue.h> - mq_open, mq_send, mq_receive, mq_getattr, mq_setattr, mq_close, mq_unlink, mq_notify

<h3>Zadanie 1. System V (50%)</h3>

Napisz prosty program typu klient-serwer, w którym komunikacja zrealizowana jest za pomocą kolejek komunikatów Systemu V - jedna, na zlecenia klientów dla serwera, druga, prywatna, na odpowiedzi.

Serwer po uruchomieniu tworzy nową kolejkę komunikatów Za pomocą tej kolejki klienci będą wysyłać komunikaty do serwera. Wysyłane zlecenia mają zawierać rodzaj zlecenia jako rodzaj komunikatu oraz informację od którego klienta zostały wysłane (PID procesu klienta), w odpowiedzi rodzajem komunikatu ma być informacja identyfikująca czekającego na nią klienta.

Klient bezpośrednio po uruchomieniu tworzy kolejkę z unikalnym kluczem IPC  i wysyła jej klucz komunikatem do serwera. Po otrzymaniu takiego komunikatu, serwer otwiera kolejkę klienta, przydziela klientowi identyfikator (np. numer w kolejności zgłoszeń) i odsyła ten identyfikator do klienta (komunikacja w kierunku serwer->klient odbywa się za pomocą kolejki klienta). Po otrzymaniu identyfikatora, klient rozpoczyna wysyłanie zleceń do serwera (w pętli), zlecenia są czytane ze standardowego wyjścia w postaci typ_komunikatu albo z pliku tekstowego w którym w każdej linii znajduje się jeden komunikat.

<b>Rodzaje zleceń</b>
<ul>
    <li>Usługa czasu (TIME): Po odebraniu takiego zlecenia serwer wysyła do klienta datę i godzinę w postaci łańcucha znaków. Klient po odebraniu informacji wysyła ją na standardowe wyjście.
    Nakaz zakończenia (END):
    <li>Po odebraniu takiego zlecenia serwer zakończy działanie, jak tylko opróżni się kolejka zleceń (zostaną wykonane wszystkie pozostałe zlecenia).
    Klient nie czeka na odpowiedź.
</ul>
Poszczególne rodzaje komunikatów należy identyfikować za pomocą typów komunikatów systemu V. Klucze dla kolejek mają być generowane na podstawie ścieżki $HOME. Małe liczby do wygenerowania kluczy oraz rodzaje komunikatów mają być zdefiniowane we wspólnym pliku nagłówkowym. Dla uproszczenia można założyć, że długość komunikatu (lub maksymalna długość łańcucha znaków przy usłudze lustra) jest ograniczona pewną stałą (jej definicja powinna znaleźć się w pliku nagłówkowym).
Klient i serwer należy napisać w postaci osobnych programów (nie korzystamy z funkcji fork). Serwer musi być w stanie pracować z wieloma klientami naraz. Przed zakończeniem pracy każdy proces powinien usunąć kolejkę którą utworzył (patrz IPC_RMID oraz funkcja atexit). Dla uproszczenia można przyjąć, że serwer przechowuje informacje o klientach w statycznej tablicy (rozmiar tablicy ogranicza liczbę klientów, którzy mogą się zgłosić do serwera).
Nalezy obsłużyć przerwanie działania serwera lub klienta za pomocą CTRL+C. W przypadku klienta, można zdefiniować dodatkowy typ komunikatu STOP, który wysyła klient, kiedy kończy pracę, aby serwer mógł skasowac jego kolejkę.
<h3>Zadanie 2. POSIX (50%)</h3>

Zrealizuj zadanie analogiczne do Zadania 1, wykorzystując kolejki komunikatów POSIX.

Kolejka klienta powinna mieć losową nazwę zgodną z wymaganiami stawianymi przez POSIX. Na typ komunikatu można zarezerwować pierwszy bajt jego treści. Przed zakończeniem pracy klient wysyła do serwera komunikat informujący, że serwer powinien zamknąć po swojej stronie kolejkę klienta. Następnie klient zamyka i usuwa swoją kolejkę. Serwer przed zakończeniem pracy zamyka wszystkie otwarte kolejki i usuwa kolejkę, którą utworzył.
