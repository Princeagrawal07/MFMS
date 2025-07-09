#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <limits>
#include <fstream>
#include <sstream>
#include <ctime>
#include <stack>
#include<windows.h>

using namespace std;

const string ADMIN_USERNAME = "JIIT";
const string ADMIN_PASSWORD = "HELLOJAYPEE";

const double BASE_FARE = 20.0;
const double FARE_PER_KM = 2.0;

const double CHILD_DISCOUNT = 0.5;
const double SENIOR_DISCOUNT = 0.3;
const double FREQUENT_TRAVELER_DISCOUNT = 0.2;

const double PEAK_HOUR_MULTIPLIER = 1.5;
const double NON_PEAK_MULTIPLIER = 1.0;

const string TRAVELLER_FILE = "travellers.txt";

map<string, vector<pair<string, double>>> metroGraph;

class Traveller
{
public:
    string username;
    string password;
    string birthDate;
    double balance;
};


int calculateAge(string& birthDate)
{
    time_t now = time(0);
    tm* currentDate = localtime(&now);
    if (!currentDate)
    {
        throw runtime_error("Failed to get current date");
    }
    int currentYear = currentDate->tm_year + 1900;
    int currentMonth = currentDate->tm_mon + 1;
    int currentDay = currentDate->tm_mday;
    int birthYear, birthMonth, birthDay;
    if (sscanf(birthDate.c_str(), "%d-%d-%d", &birthYear, &birthMonth, &birthDay) != 3)
    {
        throw invalid_argument("Invalid date format. Expected YYYY-MM-DD");
    }
    if (birthYear > currentYear ||
        (birthYear == currentYear && birthMonth > currentMonth) ||
        (birthYear == currentYear && birthMonth == currentMonth && birthDay > currentDay))
    {
        throw invalid_argument("Birth date is in the future");
    }
    int age = currentYear - birthYear;
    if (currentMonth < birthMonth || (currentMonth == birthMonth && currentDay < birthDay))
    {
        age--;
    }
    return age;
}

map<string, Traveller> loadTravellers()
{
    map<string, Traveller> travellers;
    ifstream file(TRAVELLER_FILE);

    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            stringstream ss(line);
            string username, password, birthDate;
            double balance;

            ss >> username >> password >> birthDate >> balance;
            travellers[username] = {username, password, birthDate, balance};
        }
        file.close();
    }

    return travellers;
}

void saveTravellers(const map<string, Traveller>& travellers)
{
    ofstream file(TRAVELLER_FILE);

    if (file.is_open())
    {
        for (auto& traveller : travellers)
        {
            file << traveller.second.username << " " << traveller.second.password << " "
                 << traveller.second.birthDate << " " << traveller.second.balance << endl;
        }
        file.close();
    }
}

void addConnection(string station1, string station2, double distance)
{
    transform(station1.begin(), station1.end(), station1.begin(), ::toupper);
    transform(station2.begin(), station2.end(), station2.begin(), ::toupper);

    metroGraph[station1].push_back({station2, distance});
    metroGraph[station2].push_back({station1, distance});
}

void removeConnection(string station1, string station2)
{
    transform(station1.begin(), station1.end(), station1.begin(), ::toupper);
    transform(station2.begin(), station2.end(), station2.begin(), ::toupper);
    for (auto it = metroGraph[station1].begin(); it != metroGraph[station1].end(); ++it)
    {
        if (it->first == station2)
        {
            metroGraph[station1].erase(it);
            break;
        }
    }
    for (auto it = metroGraph[station2].begin(); it != metroGraph[station2].end(); ++it)
    {
        if (it->first == station1)
        {
            metroGraph[station2].erase(it);
            break;
        }
    }
    cout << "Connection removed between " << station1 << " and " << station2 << "." << endl;
}

void displayNetwork()
{
    const int width = 60; // Adjust width based on console or longest line
    cout << setw(width) << "DELHI METRO NETWORK" << endl;
    cout << endl;
    for (auto& station : metroGraph)
    {
        cout << station.first << " -> ";
        for (auto& neighbor : station.second)
        {
            cout << "(" << neighbor.first << ", " << neighbor.second << " km) ";
        }
        cout << endl;
    }
}

vector<string> findShortestPath(const string& start, const string& destination)
{
    map<string, double> dist;
    map<string, string> parent;
    priority_queue<pair<double, string>, vector<pair<double, string>>, greater<>> pq;

    for (const auto& station : metroGraph)
    {
        dist[station.first] = numeric_limits<double>::infinity();
    }
    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty())
    {
        string current = pq.top().second;
        double currentDist = pq.top().first;
        pq.pop();

        if (current == destination) break;

        for (const auto& neighbor : metroGraph[current])
        {
            string nextStation = neighbor.first;
            double weight = neighbor.second;
            double newDist = dist[current] + weight;

            if (newDist < dist[nextStation])
            {
                dist[nextStation] = newDist;
                parent[nextStation] = current;
                pq.push({newDist, nextStation});
            }
        }
    }

    vector<string> path;
    if (dist[destination] == numeric_limits<double>::infinity())
    {
        return path;
    }

    string current = destination;
    while (current != start)
    {
        path.push_back(current);
        current = parent[current];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    return path;
}

bool isPeakHour(int hour)
{
    return (hour >= 7 && hour < 10) || (hour >= 17 && hour < 20);
}

double calculatePathFare(const vector<string>& path, int age, bool isLoggedIn, int hour)
{
    double totalDistance = 0;
    for (size_t i = 0; i < path.size() - 1; i++)
    {
        string current = path[i];
        string next = path[i + 1];
        for (auto& neighbor : metroGraph[current])
       {
            if (neighbor.first == next)
            {
                totalDistance += neighbor.second;
                break;
            }
        }
    }
    double baseFare = BASE_FARE + (totalDistance * FARE_PER_KM);
    if (isPeakHour(hour))
    {
        baseFare *= PEAK_HOUR_MULTIPLIER;
    } else {
        baseFare *= NON_PEAK_MULTIPLIER;
    }
    if (isLoggedIn)
    {
        baseFare *= (1 - FREQUENT_TRAVELER_DISCOUNT);
    }
    if (age < 12)
    {
        baseFare *= (1 - CHILD_DISCOUNT);
    } else if (age >= 60) {
        baseFare *= (1 - SENIOR_DISCOUNT);
    }
    return baseFare;
}

Traveller travellerLogin(map<string, Traveller>& travellers)
{
    string username, password;
    cout << "Enter your USERNAME: ";
    cin >> username;
    cout << "Enter your PASSWORD: ";
    cin >> password;

    if (travellers.find(username) != travellers.end())
    {
        if (travellers[username].password == password)
        {
            cout << "Login successful!" << endl;
            return travellers[username];
        }
        else
        {
            cout << "Invalid password." << endl;
            return {"", "", "", -1};
        }
    }
    else
    {
        cout << "Username not found. Registering as a new traveller." << endl;
        string birthDate;
        while (true)
        {
            cout << "Enter your birth date (YYYY-MM-DD): ";
            cin >> birthDate;
            try
            {
                int age = calculateAge(birthDate);
                break;
            }
            catch (const invalid_argument& e)
            {
                cout << "Invalid birth date: " << e.what() << ". Please try again." << endl;
            }
            catch (const runtime_error& e)
            {
                cout << "Error: " << e.what() << ". Please try again." << endl;
            }
        }
        travellers[username] = {username, password, birthDate, 0.0};
        saveTravellers(travellers);
        cout << "Registration successful! Your balance is $0." << endl;

        return travellers[username];
    }
}

void nonLoggedInMenu()
{
    string start, destination;
    int hour;
    cout << "Welcome, Guest!" << endl;
    cout << "Available stations: ";
    for (auto& station : metroGraph)
    {
        cout << station.first << ", ";
    }
    cout << "\b\b  " << endl;

    cout << "Enter your starting station: ";
    cin >> start;
    cout << "Enter your destination station: ";
    cin >> destination;

    transform(start.begin(), start.end(), start.begin(), ::toupper);
    transform(destination.begin(), destination.end(), destination.begin(), ::toupper);

    if (metroGraph.find(start) == metroGraph.end() || metroGraph.find(destination) == metroGraph.end())
    {
        cout << "Error: One or both stations (" << start << ", " << destination << ") do not exist in the metro network." << endl;
        return;
    }

    if (start == destination)
    {
        cout << "Start and destination stations cannot be the same." << endl;
        return;
    }

    cout << "Enter the current hour (0-23): ";
    cin >> hour;

    if (hour < 0 || hour > 23)
    {
        cout << "Invalid hour. Please enter a value between 0 and 23." << endl;
        return;
    }

    vector<string> shortestPath = findShortestPath(start, destination);
    if (shortestPath.empty())
    {
        cout << "No path exists between " << start << " and " << destination << "." << endl;
        return;
    }

    cout << "\nShortest Path: ";
    for (const auto& station : shortestPath)
    {
        cout << station << " -> ";
    }

    cout << "\b\b  ";
    double fare = calculatePathFare(shortestPath, 25, false, hour);

    cout << " | Fare: $" << fare << endl;

    cout << "Your total fare is: $" << fare << endl;
}

void travellerMenu(Traveller& traveller, map<string, Traveller>& travellers)
{
    int choice;

    while (true)
        {
        cout << "\nWelcome, " << traveller.username << "!" << endl;
        cout << "1. Calculate Fare" << endl;
        cout << "2. Add Balance" << endl;
        cout << "3. Check Balance" << endl;
        cout << "4. Logout" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == 1)
        {
            string start, destination;
            int hour;

            cout << "Available stations: ";
            for (auto& station : metroGraph)
            {
                cout << station.first << ", ";
            }
            cout << "\b\b  " << endl;

            cout << "Enter your starting station: ";
            cin >> start;
            cout << "Enter your destination station: ";
            cin >> destination;

            transform(start.begin(), start.end(), start.begin(), ::toupper);
            transform(destination.begin(), destination.end(), destination.begin(), ::toupper);

            if (metroGraph.find(start) == metroGraph.end() || metroGraph.find(destination) == metroGraph.end())
            {
                cout << "Error: One or both stations (" << start << ", " << destination << ") do not exist in the metro network." << endl;
                continue;
            }

            if (start == destination)
            {
                cout << "Start and destination stations cannot be the same." << endl;
                continue;
            }

            cout << "Enter the current hour (0-23): ";
            cin >> hour;

            if (hour < 0 || hour > 23)
            {
                cout << "Invalid hour. Please enter a value between 0 and 23." << endl;
                continue;
            }

            vector<string> shortestPath = findShortestPath(start, destination);
            if (shortestPath.empty())
            {
                cout << "No path exists between " << start << " and " << destination << "." << endl;
                continue;
            }

            cout << "\nShortest Path: ";
            for (const auto& station : shortestPath)
            {
                cout << station << " -> ";
            }
            cout << "\b\b  ";
            double fare = calculatePathFare(shortestPath, calculateAge(traveller.birthDate), true, hour);
            cout << " | Fare: $" << fare << endl;

            cout << "Your total fare is: $" << fare << endl;
            if (traveller.balance >= fare)
            {
                traveller.balance -= fare;
                travellers[traveller.username].balance = traveller.balance;
                saveTravellers(travellers);
                cout << "Fare deducted. Remaining balance: $" << traveller.balance << endl;

            }
            else
            {
                cout << "Insufficient balance. Please add balance." << endl;
            }
        }
         else if (choice == 2)
        {
            double amount;
            cout << "Enter the amount to add: $";
            cin >> amount;

            traveller.balance += amount;
            travellers[traveller.username].balance = traveller.balance;
            saveTravellers(travellers);

            cout << "Balance updated. New balance: $" << traveller.balance << endl;
        }
        else if (choice == 3)
        {
            cout << "Your current balance is: $" << traveller.balance << endl;
        }
        else if (choice == 4)
        {
            cout << "Logging out." << endl;
            break;
        }
        else
        {
            cout << "Invalid choice. Try again." << endl;
        }
    }
}

void dfs(const string& station, map<string, bool>& visited, const map<string, vector<pair<string, double>>>& graph)
{
    visited[station] = true;
    for (auto& neighbor : graph.at(station))
    {
        if (!visited[neighbor.first])
        {
            dfs(neighbor.first, visited, graph);
        }
    }
}

bool isArticulationPoint(const string& station)
{
    map<string, bool> visited;
    int originalComponents = 0;

    for (auto& node : metroGraph)
    {
        if (!visited[node.first])
        {
            dfs(node.first, visited, metroGraph);
            originalComponents++;
        }
    }
    map<string, vector<pair<string, double>>> tempGraph = metroGraph;
    tempGraph.erase(station);
    for (auto& node : tempGraph)
    {
        auto& neighbors = node.second;
        neighbors.erase(remove_if(neighbors.begin(), neighbors.end(),[&station](const pair<string, double>& edge)
           {
               return edge.first == station;
            }), neighbors.end());
    }
    visited.clear();
    int newComponents = 0;

    for (auto& node : tempGraph)
    {
        if (!visited[node.first])
        {
            dfs(node.first, visited, tempGraph);
            newComponents++;
        }
    }

    return newComponents > originalComponents;
}

bool bfs(const map<string, map<string, double>>& graph, const string& source, const string& sink, map<string, string>& parent)
{
    queue<string> q;
    q.push(source);
    parent[source] = "";

    while (!q.empty())
    {
        string u = q.front();
        q.pop();

        for (auto& edge : graph.at(u))
        {
            string v = edge.first;
            double capacity = edge.second;

            if (parent.find(v) == parent.end() && capacity > 0)
            {
                parent[v] = u;
                if (v == sink)
                    return true;

                q.push(v);
            }
        }
    }
    return false;
}

double edmondsKarp(map<string, map<string, double>>& graph, const string& source, const string& sink)
{
    double maxFlow = 0.0;
    map<string, map<string, double>> residualGraph = graph;
    map<string, string> parent;

    while (bfs(residualGraph, source, sink, parent))
        {
        double pathFlow = numeric_limits<double>::max();
        for (string v = sink; v != source; v = parent[v])
        {
            string u = parent[v];
            pathFlow = min(pathFlow, residualGraph[u][v]);
        }

        for (string v = sink; v != source; v = parent[v])
        {
            string u = parent[v];
            residualGraph[u][v] -= pathFlow;
            residualGraph[v][u] += pathFlow;
        }

        maxFlow += pathFlow;
        parent.clear();
    }
    return maxFlow;
}

bool adminLogin()
{
    string username, password;
    cout << "Enter admin username: ";
    cin >> username;
    cout << "Enter admin password: ";
    cin >> password;

    if (username == ADMIN_USERNAME && password == ADMIN_PASSWORD)
    {
        cout << "Admin login successful!" << endl;
        return true;
    } else
    {
        cout << "Invalid admin credentials. Access denied." << endl;
        return false;
    }
}

void adminMenu()
{
    int choice;

    while (true)
    {
        cout << "\nAdmin Menu:" << endl;
        cout << "1. Add Connection" << endl;
        cout << "2. Remove Connection" << endl;
        cout << "3. Display Network" << endl;
        cout << "4. Check Articulation Point" << endl;
        cout << "5. Calculate Maximum Flow" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        string station1, station2;
        double distance;
        switch (choice)
        {
            case 1:
                cout << "Enter the first station: ";
                cin >> station1;
                cout << "Enter the second station: ";
                cin >> station2;
                cout << "Enter the distance between " << station1 << " and " << station2 << " (in km): ";
                cin >> distance;
                addConnection(station1, station2, distance);
                break;

            case 2:
                cout << "Enter the first station: ";
                cin >> station1;
                cout << "Enter the second station: ";
                cin >> station2;

                removeConnection(station1, station2);
                break;

            case 3:
                displayNetwork();
                break;

            case 4:
                cout << "Enter the station to check: ";
                cin >> station1;
                transform(station1.begin(), station1.end(), station1.begin(), ::toupper);
                if (metroGraph.find(station1) == metroGraph.end())
                {
                    cout << "Station " << station1 << " does not exist." << endl;
                }
                else if (isArticulationPoint(station1))
                {
                    cout << "Station " << station1 << " is an articulation point. Removing it will disconnect the graph." << endl;
                }
                else
                {
                    cout << "Station " << station1 << " is not an articulation point. Removing it will not disconnect the graph." << endl;
                }
                break;
            case 5:
                cout << "Enter the source station: ";
                cin >> station1;
                cout << "Enter the sink station: ";
                cin >> station2;

                transform(station1.begin(), station1.end(), station1.begin(), ::toupper);
                transform(station2.begin(), station2.end(), station2.begin(), ::toupper);
                if (metroGraph.find(station1) == metroGraph.end() || metroGraph.find(station2) == metroGraph.end())
                {
                    cout << "One or both stations do not exist." << endl;
                }
                else
                {
                    map<string, map<string, double>> flowGraph;
                    for (auto& station : metroGraph)
                    {
                        for (auto& neighbor : station.second)
                        {
                            flowGraph[station.first][neighbor.first] = neighbor.second;
                        }
                    }
                    double maxFlow = edmondsKarp(flowGraph, station1, station2);
                    cout << "The maximum number of passengers that can travel from " << station1 << " to " << station2 << " is: " << maxFlow << " per hour." << endl;
                }
                break;
            case 6:
                cout << "Exiting admin menu." << endl;
                return;
            default:
                cout << "Invalid choice. Try again." << endl;
        }
    }
}

int main()
{
    system("color B0" );  //used for background color

    addConnection("NoidaElectronicCity", "RajivChowk", 5.0);
    addConnection("RajivChowk", "DwarkaSector21", 3.0);
    addConnection("DwarkaSector21", "KarolBagh", 4.0);
    addConnection("NOIDAElectronicCity", "DwarkaSector21", 6.0);
    addConnection("RajivChowk", "KarolBagh", 9.0);
    addConnection("KarolBagh", "SupremeCourt", 7.0);
    addConnection("SupremeCourt", "Akshardham", 2.0);

    map<string, Traveller> travellers = loadTravellers();
    int userType;

    while (true)
    {
        displayNetwork();

        cout<<endl;
        cout << "Welcome to the Metro Fare Calculator!" << endl;
        cout << "1. Admin" << endl;
        cout << "2. Traveller Login" << endl;
        cout << "3. Guest (Non-Logged-In)" << endl;
        cout << "4. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> userType;

        if (userType == 1)
        {
            if (adminLogin())
            {
                adminMenu();
            }
        }
        else if (userType == 2)
        {
            Traveller traveller = travellerLogin(travellers);
            if (traveller.username != "")
            {
                travellerMenu(traveller, travellers);
            }
        }
        else if (userType == 3)
        {
            nonLoggedInMenu();
        }
        else if (userType == 4)
        {
            cout << "Exiting program. Goodbye!" << endl;
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again." << endl;
        }
    }
    return 0;
}
