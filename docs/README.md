# Dokumentace k zápočtovému programu

**Projekt:** HTTP server knihovna \
**Autor:** Oliver Řezníček \
**Datum vzniku:** 2025/26

## Přesné zadání

Přesné zadání lze nalézt zde [odkaz](/project/docs/project_proposal.md).

## Uživatelská dokumentace

### Sestavení knihovny

Knihovna se sestaví následujícím způsobem. Je potřeba mít nainstalovaný program `CMake`. \
Uvnitř složky `project` se spustí následující sekvence příkazů:


```
mkdir <BUILD-DIR>
cd <BUILD-DIR>
cmake ..
```

To vytvoří potřebnou konfiguraci pro sestavení a poté je na výběr ze tří možností:
 1. `make` sestaví pouze knihovnu v statické podobě
 2. `make tests` sestaví knihovnu i se sadou testů, které se následně dají spustit hromadně pomocí `ctest` nebo jednotlivě - nalezením spuštěním daného testu (např. `<BUILD-DIR>/tests/parser_test`)
 š. `make examples` sestaví pár příkladů nacházejících se ve složce examples. Každý z nich demonstruje jinou feature knihovny

## Použití knihovny

Nejjednodušší použití knihovny vypadá následovně:

```c++
#include <http/server.hpp>

int main()
{
    net::Context context;
    http::Server srv = http::ServerBuilder().build();
    srv.run();
}
```

Tyto 3 řádky dělají následující:
 - `net::Context` se stará o to, aby se při použití na Windows dobře inicializovala `winsock2` knihovna
 - druhý řádek dělá všechnu "magii"
   - třída `http::ServerBuilder` se stará o konfiguraci dané instance serveru, pokud se na třídě před voláním `.build()` nevolá žádná jiná funkce tvaru `set_...()` nebo `enable_...()` či `disable_...()`, použije se [výchozí konfigurace](#vychozi-konfigurace). Funkce `.build()` pak vytvoří instanci `http::Server`, které se daná konfigurace předá.
 - třetí řádek pak server spustí a s takto vypadající `main()` funkcí se dá běžící process zabít jedině interruptem (Ctrl+c)

Server by se dal také zastavit pěkněji, pokud by se spustil v jiném než hlavním vlákně:

```c++
int main() {
    std::jthread srv_thread([&srv]() {
        srv.run();
    });
    // Odsud se dá serveru poslat request
    srv.stop();
    // srv.stop() přepne vnitří flag serveru
    // který způsobí vyskočení z hlavní smyčky
}
// po konci scopu main() funkce se v jthread pokusí
// automaticky najoinovat, což se mu povede díky
// doběhnutí funkce srv.run()
```

Tohoto způsobu spuštění využívám v testech.

### Konfigurace

Konfigurace serveru se dělá tedy pomocí třídy `http::ServerBuilder`, která má následující rozhraní:

```c++
class ServerBuilder {
private:
    // ...
public:
    // Nastavuje veřejnou složku, ze které bude server číst soubory (nebo do ní zapisovat) a vracet je klientovi
    ServerBuilder& set_public_dir(const std::filesystem::path&);

    // Nastavuje vzor pro chybovou stránku
    // Na vstupu bere jako řetězec obsah, který se bude v
    // těle response vrace klientovi 
    // Očekává se, že stránka bude typu text/html
    // Pro vkládání chybového kódu se použivá placeholder {0}
    // Pro vkládání chybové hlášky se používá placeholder  {1}
    // Všechny jednoduché složené závorky musí být zdvojeny,
    // aby se ve výsledné stránce zobrazily.
    // Důvodem je použité funkce std::format
    ServerBuilder& set_error_page_template(std::string&&);

    // Directory listing umožňuje zobrazit obsah
    // podsložky public složky pokud v ní chybí
    // soubor index.html
    ServerBuilder& enable_directory_listing();
    ServerBuilder& disable_directory_listing();

    // Přidání custom handler funkce, která se zavolá na dané
    // pokud server dostane danou relativní cestu.
    // Je zodpovědností uživatele této funkce dá cestu ve tvaru, který je relativní (nezačíná symbolem '/')
    // zároveň musí být cesta normalizovaná
    // good: api/create-user
    // wrong: /api/.././//.//create-user
    ServerBuilder& add_route(std::string&& path, http::RequestMethod method, handlers::Router::HandlerFunc&& func);

    // Nastavuje port, na kterém server poběží
    ServerBuilder& set_port(in_port_t);

    // Zapíná IPv4 přístup k serveru
    ServerBuilder& enable_ipv4();
    ServerBuilder& disable_ipv4();

    // Zapíná IPv6 přístup k serveru
    ServerBuilder& enable_ipv6();
    ServerBuilder& disable_ipv6();

    // Nastavení limitů pro server
    ServerBuilder& set_request_headers_size_limit(std::size_t);
    ServerBuilder& set_request_body_size_limit(std::size_t);
    ServerBuilder& set_request_target_size_limit(std::size_t);

    // Nastavení délky timeoutu
    ServerBuilder& set_request_timeout(const timeval& timeout);

    // Počet vláken, tedy maximální počet klientů, které je server schopen obsluhovat v jednom okamžiku
    ServerBuilder& set_thread_count(std::size_t);

    // Vytvoří instanci serveru s danou konfigurací
    Server build();
};
```

#### Výchozí konfigurace

 - public je složka, ze které je server spuštěn v command-line
 - výchozí error stránka:

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Error {0}</title>
    <style>
        body {{
            font-family: sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #f9f9f9;
            color: #333;
            text-align: center;
        }}
        h1 {{ margin: 0; font-size: 8rem; color: #e74c3c; }}
        p {{ margin: 0; font-size: 3rem; color: #666; }}
    </style>
</head>
<body>
    <div>
        <h1>{0}</h1>
        <p>{1}</p>
    </div>
</body>
</html>
```

 - Directory listing je vypnutý
 - Port: **8080**
 - Server je **dual-stack**, tedy podporuje IPv4 i IPv6 připojení
 - Limity:
   - Headers: 8 KiB
   - Body: 4 MiB
   - Request Target: 8 KiB
 - Timeout: 5 sekund
 - Thread count: roven počtu jader procesoru

#### Nastavení logu

Logging je jediná feature knihovny, která se nezapíná přes `http::ServerBuilder`. Má následující rozhraní:

```c++
namespace logger {

enum class Level {
    Error,
    Warning,
    Info,
    Debug
};

struct Options {
    bool show_timestamp = true;
    bool show_thread_id = true;
};

// Pomocné funkce pro psaní do logu
template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args);
template <typename... Args>
void warning(std::format_string<Args...> fmt, Args&&... args)
template <typename... Args>
void info(std::format_string<Args...> fmt, Args&&... args)
template <typename... Args>
void debug(std::format_string<Args...> fmt, Args&&... args);

// Nastavuje jaké informace navíc se mají do logu vypisovat
void set_options(Options&& options);

// Nastavuje úroveň logování, ty jsou 4
// Výchozí úroveň je Info, což znamená, že se nevypisují pouze
// zprávy typu Debug, ale zbylé Error, Warning a Info
// se vypisují
void set_level(Level new_level);

// Pokud chce uživatel přesměrovat vypisování jinam než
// na standartní výstup, který je výchozím bodem pro logging,
// poskytne absolutní cestu, kde bude bydlet logovací soubor
void set_log_file(const std::string& filepath);

} // end of `log` namespace
```

## Programátorská dokumentace

Pokud je konfigurace serveru v pořádku, tak se uvnitč `http::ServerBuilder::build()` zavolá konstruktor serveru, kde se vykrade veškerá konfigurace a předá na správná místa a tím nejhlavnějším je, že se vytvoří `ServerSocket`, který naslouchá na dané adrese a čeká až se někdo připojí.

Vstupním bodem knihovny je cyklus uvnitř funkce `http::Server::run()`:

```c++
while (is_running_) {
    SocketAddr6 client_addr;
    // Funkce poll se podívá, jestli se nepřipojil nějaký klient (na to čeká nanejvýš 100ms):
    //  - pokud ne:
    //    - vratí invalid `ClientSocket`
    //  - pokud ano
    //    - poskytne nám klienta, kterého můžeme obsloužit
    ClientSocket csock = ssock_.poll(client_addr, &timeout_);
    if (!csock.is_valid()) {
        continue;
    }
    // V tomto bodě přidáme funkci na obsloužení klienta do thread-safe fronty
    // Nějaký volný worker thread si tuto prácí vyzvedne a request zpracuje
    // Pokud neexistuje volný worker thread, tak musí Task sedět ve frontě
    pool_.submit_task([this, conn = Connection(std::move(csock))]() mutable {
        handle_client(std::move(conn));
    });
}
```

Velmi důležitá je třída `http::Connection`, která pomocí dvou funkcí zapouzdřuje `ClientSocket` a umožňuje pohodlnější čtení dat, která nám přijdou po síti. Ty funkce vypadají následovně:

```c++
class Connection {    
    // Čte data ze sítě než v nich potká delimiter nebo data překročí maximální velikost max_bytes
    std::expected<std::string, StatusCode> read_until(std::string_view delimiter, std::size_t max_bytes);
    // Přečte daný počet bytů ze sítě 
    std::expected<std::string, StatusCode> read(std::size_t bytes);
}
```

V obou těchto funkcích se data čtou po rozumně velkých chunkách, abychom zamezili příliš častému volání syscallu `recv`.

Takřka srdcem knihovny by se dala nazvat privátní funkce `http::Server::handle_client(Connection&&)`:

```c++
Request request;
Response response;

try {
    // Chceme zparsovat request
    auto req = parser_.parse_request(conn);

    if (req.has_value()) {
        request = *req;
        // Request se zparsoval úspěšně ... Router bude vědět co s ním
        // Router buď najde (nebo nenajde -> 404) konkrétní soubor
        // nebo spustí custom funkci, kterou uživatel knihovny přidal pomoci ServerBuilder::add_route
        response = router_.handle_request(request);
    } else if (req.error().code == StatusCode::None) {
        return; // client closed
    } else {
        // Request se nezparsoval úspěšně
        // Vrátíme uživateli chybu
        response = Response(req.error());
    }
} catch (...) {
    log_exception(std::current_exception());
    response = Response(ServerErr(StatusCode::InternalServerError));
}

conn.send(response.to_string());
```

### Testy

Velmi zajímavou částí kódu jsou testy. V hlavičkovém souboru `tests/test_common.hpp` je hlavní makro `TEST_CASE`, pomocí kterého se vytváří testy. Ty se postupně přidávájí do třídy `TestRegistry`, která si je udržuje a ke každému testu se přilinkuje společná `main()` funkce z `tests/test_main.cpp`.

Přiznám se, že makro `TEST_CASE` jsem ukradnul z posledního úkolu na pokročilé C++, které jsem absolvoval minulý rok.

## Průběh prací

Bohužel se mi nepodařilo splnit všechno z definovaného zadání. Konkrétně:
 - keep-alive
 - running server as background daemon

Ukousnul jsem si moc velký oříšek. Aplikace typu HTTP server musí být napsaná precizně a takřka každá její část by se měla řádně otestovat. Bohužel jsem napsal testy jen k těm částem, které mi přišly důležité. Psaní testů bylo dost časově náročné.

Také mezi sebou všechny části aplikace velmi úzce souvisí. Navíc síťové chyby se někdy dost špatně ladí. Určitě jsem měl mnohem dřív přidat do projektu logging, to by mi pomohlo ze všeho nejvíc. Asi je tu u síťových aplikací standard, každou maličkost vypsat, aby člověk věděl, co se v aplikaci děje.