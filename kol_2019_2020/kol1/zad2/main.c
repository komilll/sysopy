#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

/* DODANE PRZEZE MNIE */

//Rozwiazanie razem z pierwszym cwiczeniem na 4.5
//Brak odbierania wartosci sygnalu
//Nalezy wykorzystac sygnature w potsaci: void(*) (int, siginfo_t *, void *)
//Aby przekazac wartosc w siginfo_t i ja wypisac

void au(int sig_no){
    printf("Got signal %d\n", sig_no);
}

/* KONIEC MOJEGO KODU */

int main(int argc, char* argv[]) {

    if(argc != 3){
        printf("Not a suitabele number of program parameters\n");
        return 1;
    }

    //--Zadeklarowanie handlera do sigaction
    struct sigaction action;
    action.sa_handler = au;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

	/* DODANE PRZEZE MNIE */
	
    //--Dodaje wiecej elementow, aby sprawdzic, ze sygnaly sa blokowane
    sigaction(9, &action, NULL);
    sigaction(10, &action, NULL);
    sigaction(11, &action, NULL);

    //--Blokowanie sygnalow 
    sigset_t signal_set;
    sigfillset(&signal_set);
    sigdelset(&signal_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &signal_set, NULL);
	
	/* KONIEC MOJEGO KODU */
		
    int child = fork();
    if(child == 0) {
        //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        //zdefiniuj obsluge SIGUSR1 w taki sposob zeby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
		
		/* DODANE PRZEZE MNIE */
		
        int sigNum = atoi(argv[2]);

        //--Wyslanie sygnalu z wartoscia
        union sigval val;
        int valueOfArg = atoi(argv[1]);
        val.sival_int = valueOfArg;
        sigqueue(child, sigNum, val);
		
		/* KONIEC MOJEGO KODU */
    }

    return 0;
}
