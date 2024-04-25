# Software keylogger

Projekt ma na celu poznanie podstawowe zasady działania sterowników jądra Linux. Należy samodzielnie przygotować sterownik uruchamiany jako moduł jądra Linux do obsługi podstawowej klawiatury. Sterownik powinien rejestrować wszystkie naciskane klawisze oraz zapisywać je w wybrany sposób. Sterownik powinien zawierać easter egg polegający na wykonaniu dodatkowej, wybranej przez autorów operacji po wykryciu wprowadzenia tzw. Konami Code na klawiaturze - taką operacją może być np. zagranie melodii korzystając z brzęczyka systemowego (beepera). Raport powinien zawierać szczegółowy opis działania sterownika oraz analizę działania sterowników jądra Linux w ogólnym podejściu.

# Kompilacja i instalacja modułu

1. __Kompilacja modułu:__
   Aby skompilować moduł należy użyć polecenia `make -C /lib/modules/$(uname -r)/build M=$(pwd) modules`
2. __Uruchomienie modułu:__
   Po skompilowaniu modułu, załaduj go do jądra za pomocą polecenia `sudo insmod keylogger.ko`.
   Sprawdź logi jądra, aby upewnić się, że moduł został załadowany poprawnie: `sudo dmesg`.
3. __Odczyt wciśniętych klawiszy:__
   Logi wciskanych klawiszy zapisują się w folderze /proc/keylogger. Aby wyświetlić listę wciśniętych można skorzystać z polecenia `cat /proc/keylogger`
5. __Zatrzymanie modułu:__
   Aby wyładować moduł z jądra, użyj polecenia `sudo rmmod keylogger`.
# TODO
- dodać Konami Code i easter egga :)
