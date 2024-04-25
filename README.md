# Software keylogger

Projekt ma na celu poznanie podstawowe zasady działania sterowników jądra Linux. Należy samodzielnie przygotować sterownik uruchamiany jako moduł jądra Linux do obsługi podstawowej klawiatury. Sterownik powinien rejestrować wszystkie naciskane klawisze oraz zapisywać je w wybrany sposób. Sterownik powinien zawierać easter egg polegający na wykonaniu dodatkowej, wybranej przez autorów operacji po wykryciu wprowadzenia tzw. Konami Code na klawiaturze - taką operacją może być np. zagranie melodii korzystając z brzęczyka systemowego (beepera). Raport powinien zawierać szczegółowy opis działania sterownika oraz analizę działania sterowników jądra Linux w ogólnym podejściu.

# Kompilacja i instalacja modułu

1. Kompilacja modułu:
   Upewnij się, że masz zainstalowane narzędzia do kompilacji jądra, takie jak nagłówki jądra Linuxa i pakiet build-essential.
   Użyj polecenia make w katalogu, w którym znajdują się pliki modułu (keylogger.c, keylogger.h, Makefile).
2. Uruchomienie modułu:
   Po skompilowaniu modułu, załaduj go do jądra za pomocą polecenia insmod keylogger.ko.
   Sprawdź logi jądra, aby upewnić się, że moduł został załadowany poprawnie: dmesg.
3. Wyładowanie modułu:
   Aby wyładować moduł z jądra, użyj polecenia rmmod keylogger.
