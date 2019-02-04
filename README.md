# Połączenie klient-serwer

### Kompilacja i uruchomienie

Skrypt car.sh (compile and execute) kompiluje i uruchamia proces serwera. Jako jedyny parametr przyjmuje ilość dodatkowych procesów klienta (brak oznacza 0).

### Protokół połączenia

```c
#define M 1000
struct srvquery{
    long type;
    long ip;
    char message[M];
} query;

struct srvresponse{
    long type;
    char message[M];
} response;
```
- sygnał = 10: rejestracja "<login>|<hasło>"
- sygnał = 11: logowanie "<login>|<hasło>"
- sygnał = 12: stworzenie tematu "<nazwa tematu>"
- sygnał = 13: przesłanie wiadomości do tematu "<nazwa tematu>|<treść wiadomości>"
- sygnał = 14: odczytanie wiadomości z tematu "<nazwa tematu>"
- sygnał = 15: zasubskrybowanie tematu "<nazwa tematu>"

### Opis programu

Plik inf136714_c.c zawiera program serwera, który oczekuje na wiadomość i po jej uzyskaniu, następnie przetworzeniu zawsze odsyła odpowiedź na podany adres ip.

Plik inf136714_k.c zawiera program klienta, który oczekuje na instrukcje użytkownika i wysyła odpowiednią do polecenia wiadomość do serwera oraz oczekuje na odpowiedź.
