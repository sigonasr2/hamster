#include <HamsterNet.h>
#include <json.hpp>

#ifdef __EMSCRIPTEN__

EM_JS(int, hamsterNet__initSession, (), {
    return Asyncify.handleSleep(function(wakeUp) {
        fetch('/session', { method: 'POST', credentials: 'same-origin' })
            .then((response) =>
            {
                return response.ok
                ? response.json().then((data) => JSON.stringify(data, null, 2))
                : Promise.reject(new Error("Unexpected response"));
            })
            .then((message) =>
            {
                console.log(message);
                wakeUp(1);
            })
            .catch((err) =>
            {
                console.error(err.message);
                wakeUp(0);
            })    
    });
});

EM_JS(int, hamsterNet__setRacerName, (const char* str), {
    
    let racerName = UTF8ToString(str);
    
    return Asyncify.handleSleep(function(wakeUp) {
        fetch('/name', {
            method: 'POST',
            credentials: 'same-origin',
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({ userName: racerName })
        }).then((response) =>
        {
            return response.ok
            ? response.json().then((data) => JSON.stringify(data, null, 2))
            : Promise.reject(new Error("Unexpected response"));
        })
        .then((message) =>
        {
            wakeUp(1);
        })
        .catch((err) =>
        {
            console.error(err.message);
            wakeUp(0);
        })    
    });
});

EM_JS(int, hamsterNet__startRace, (), {
    
    return Asyncify.handleSleep(function(wakeUp) {
        fetch('/race', {
            method: 'POST',
            credentials: 'same-origin',
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify({})
        }).then((response) =>
        {
            return response.ok
            ? response.json()
            : Promise.reject(new Error("Unexpected response"));
        })
        .then((message) =>
        {
            Module.hamsterRaceId = message.raceId;
            wakeUp(1);
        })
        .catch((err) =>
        {
            console.error(err.message);
            wakeUp(0);
        })    
    });
});

EM_JS(int, hamsterNet__finishRace, (const char* raceMap, const char* raceColor, int raceTime), {
    
    if(Module.hamsterRaceId === undefined)
    {
        console.error("Trying to finish a race that never start!");
        return 0;
    }
    
    const raceData = {
        raceId: Module.hamsterRaceId,
        raceTime: raceTime,
        raceMap: UTF8ToString(raceMap),
        raceColor: UTF8ToString(raceColor),
    };
    
    return Asyncify.handleSleep(function(wakeUp) {
        fetch('/race', {
            method: 'PATCH',
            credentials: 'same-origin',
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify(raceData)
        }).then((response) =>
        {
            return response.ok
            ? response.json().then((data) => JSON.stringify(data, null, 2))
            : Promise.reject(new Error("Unexpected response"));
        })
        .then((message) =>
        {
            wakeUp(1);
        })
        .catch((err) =>
        {
            console.error(err.message);
            wakeUp(0);
        })    
    });
});

EM_JS(int, hamsterNet__getLeaderboard, (const char* map, const char* sortBy, const int offset, const int limit, const int ascending), {

    return Asyncify.handleSleep(function(wakeUp) {
        // just some rudimentary input validation
        const params = {
            sort: ((ascending == 1) ? "asc" : "desc"),
            map: encodeURIComponent(UTF8ToString(map)),
            offset: offset,
            limit: limit,
            sortBy: "id"
        };

        if(['id', 'color', 'map', 'time', 'created_at'].indexOf(UTF8ToString(sortBy)) !== -1)
        {
            params.sortBy = UTF8ToString(sortBy);
        }

        fetch(`/race?map=${params.map}&sortBy=${params.sortBy}&sort=${params.sort}&offset=${params.offset}&limit=${params.limit}`, {
            method: 'GET',
            credentials: 'same-origin',
        }).then((response) =>
        {
            return response.ok
            ? response.json()
            : Promise.reject(new Error("Unexpected response"));
        })
        .then((message) =>
        {
            // set aside the results, so we can use them in the next phase.
            Module._leaderboardResults = JSON.stringify(message.results);
            wakeUp(1);
        })
        .catch((err) =>
        {
            console.error(err.message);
            wakeUp(0);
        });
    });
});

#else
extern "C"
{
    int hamsterNet__initSession()
    { 
        std::cout << "hamsterNet__initSession is not implemented on this platform, artificially succeeding.\n";
        return 1;
    }
    
    int hamsterNet__setRacerName(const char* str)
    {
        std::cout << "hamsterNet__setRacerName is not implemented on this platform, artificially succeeding.\n";
        return 1;
    }
    
    int hamsterNet__startRace()
    {
        std::cout << "hamsterNet__startRace is not implemented on this platform, artificially succeeding.\n";
        return 1;
    }

    int hamsterNet__finishRace(const char* raceMap, const char* raceColor, int raceTime)
    {
        std::cout << "hamsterNet__finishRace is not implemented on this platform, artificially succeeding.\n";
        return 1;
    }
    
    int hamsterNet__getLeaderboard(const char *map, const char *sortBy, int offset, int limit, int ascending)
    {
        std::cout << "hamsterNet__getLeaderboard is not implemented on this platform, artificially succeeding.\n";
        return 1;
    }
}
#endif

HamsterNet::HamsterNet()
{ }

bool HamsterNet::InitSession()
{
    return (hamsterNet__initSession() == 1);
}

void HamsterNet::SetColor(const std::string& hamsterColor)
{
    m_color = hamsterColor;
}

bool HamsterNet::SetName(const std::string& name)
{
    m_name = name;
    return (hamsterNet__setRacerName(m_name.c_str()) == 1);
}

bool HamsterNet::StartRace(const std::string& map)
{
    m_map = map;
    m_tp1 = std::chrono::system_clock::now();
    m_tp2 = std::chrono::system_clock::now();
    return (hamsterNet__startRace() == 1);
}

bool HamsterNet::FinishRace()
{
    m_tp2 = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> duration = m_tp2 - m_tp1;
    m_time = static_cast<int>(duration.count());

    return (hamsterNet__finishRace(m_map.c_str(), m_color.c_str(), m_time) == 1);
}

std::vector<LeaderboardEntry> HamsterNet::GetLeaderboard(const std::string& map, const int offset, const int limit, const std::string& sortBy, bool ascending)
{
    std::vector<LeaderboardEntry> leaderboard;

    int result = hamsterNet__getLeaderboard(map.c_str(), sortBy.c_str(), offset, limit, (ascending) ? 1 : 0);
    
    if(result == 1)
    {
        #ifdef __EMSCRIPTEN__
        
        char* leaderboardJsonString = (char*)EM_ASM_PTR({
            
            // get the number of bytes we need to allocate to fit the string
            let lengthBytes = lengthBytesUTF8(Module._leaderboardResults) + 1;

            // allocate enough memory to hold the string
            let stringOnWasmHeap = _malloc(lengthBytes);
            
            // copy the javascript string into the heap
            stringToUTF8(Module._leaderboardResults, stringOnWasmHeap, lengthBytes);
            
            // we're done with this, remove it!
            delete Module._leaderboardResults;
            
            // return the pointer
            return stringOnWasmHeap;
        });        
        
        using json = nlohmann::json;

        json leaderboardJson = json::parse(leaderboardJsonString);

        free(leaderboardJsonString);

        for(auto &el : leaderboardJson.items())
        {
            // std::string color;
            // std::string name;
            // std::string map;
            // int time;
            leaderboard.push_back(LeaderboardEntry{
                el.value().at("color"),
                el.value().at("name"),
                el.value().at("map"),
                el.value().at("time"),
            });
        }
        
        std::cout << "get leaderboard successful\n";
        
        #else
        std::cout << "HamsterNet::GetLeaderboard is not implemented for this platform\n"
        #endif
    }

    return leaderboard;
}