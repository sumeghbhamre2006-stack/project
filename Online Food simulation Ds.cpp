#include <iostream>
#include <string>
#include <list>         // Linked List: Order History
#include <stack>        // Stack: Order Basket
#include <queue>        // Queue: Delivery Queue
#include <unordered_map>// Hash Table: Menus
#include <vector>
#include <iomanip>
#include <fstream>      // For JSON file output
#include <cstdlib>
#include <ctime>
#include <sstream>      // For string manipulation in order summary
#include <algorithm>    // For std::replace (used in JSON prep)

// --- 1. CORE DATA STRUCTURES DEFINITIONS ---

struct MenuItem {
    std::string itemName;
    double price;
    int preparationTime;

    MenuItem() = default; 
    MenuItem(const std::string& name, double p, int time)
        : itemName(name), price(p), preparationTime(time) {}
};

struct Restaurant {
    std::string name;
    double rating;
    std::string cuisine;
    std::string menuId;

    Restaurant(const std::string& n, double r, const std::string& c, const std::string& id)
        : name(n), rating(r), cuisine(c), menuId(id) {}
};

struct OrderItem {
    std::string restaurantName;
    std::string itemName;
    double price;
    
    OrderItem(const std::string& rName, const std::string& iName, double p)
        : restaurantName(rName), itemName(iName), price(p) {}
};

struct DeliveryOrder {
    std::string orderId;
    std::string summary;
    double totalCost;
};

struct RatingComparator {
    bool operator()(const Restaurant& a, const Restaurant& b) const {
        return a.rating < b.rating; 
    }
};

// --- 2. BACKEND MANAGER CLASS ---

class FoodDeliveryApp {
private:
    std::list<Restaurant> restaurantDirectory;
    std::unordered_map<std::string, std::unordered_map<std::string, MenuItem>> menus;
    std::stack<OrderItem> orderBasket;
    std::priority_queue<Restaurant, std::vector<Restaurant>, RatingComparator> topRatedRestaurants;

    std::queue<DeliveryOrder> deliveryQueue;
    std::list<DeliveryOrder> orderHistory;
    
    int nextOrderId = 1001; 

// --- FINAL JSON GENERATION FUNCTION (Exports Stack, Queue, and List) ---
void generateJsonOutput() const {
    std::ofstream outfile("history.json");
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open history.json for writing.\n";
        return;
    }

    outfile << std::fixed << std::setprecision(2);
    outfile << "{\n";

    // 1. Order Basket (std::stack)
    std::stack<OrderItem> tempBasket = orderBasket;
    outfile << "  \"basket\": [\n";
    int basketCount = 0;
    while (!tempBasket.empty()) {
        OrderItem item = tempBasket.top();
        tempBasket.pop();
        if (basketCount++ > 0) { outfile << ",\n"; }
        
        std::string safe_item_name = item.itemName;
        std::replace(safe_item_name.begin(), safe_item_name.end(), '"', '\'');

        outfile << "    { \"itemName\": \"" << safe_item_name << "\", \"price\": " << item.price << " }";
    }
    outfile << "\n  ],\n";

    // 2. Delivery Queue (std::queue)
    std::queue<DeliveryOrder> tempQueue = deliveryQueue;
    outfile << "  \"queue\": [\n";
    int queueCount = 0;
    while (!tempQueue.empty()) {
        DeliveryOrder order = tempQueue.front();
        tempQueue.pop();
        if (queueCount++ > 0) { outfile << ",\n"; }

        std::string safe_summary = order.summary;
        std::replace(safe_summary.begin(), safe_summary.end(), '"', '\'');

        outfile << "    { \"orderId\": \"" << order.orderId << "\", \"totalCost\": " << order.totalCost << ", \"summary\": \"" << safe_summary << "\" }";
    }
    outfile << "\n  ],\n";

    // 3. Order History (std::list)
    outfile << "  \"history\": [\n";
    int historyCount = 0;
    for (const auto& order : orderHistory) {
        if (historyCount++ > 0) { outfile << ",\n"; }
        
        std::string safe_summary = order.summary;
        std::replace(safe_summary.begin(), safe_summary.end(), '"', '\''); 

        outfile << "    { \"orderId\": \"" << order.orderId << "\", \"totalCost\": " << order.totalCost << ", \"summary\": \"" << safe_summary << "\" }";
    }
    outfile << "\n  ]\n";

    outfile << "}\n";
    outfile.close();
}
// --- END JSON GENERATION ---


public:
    const std::list<Restaurant>& getRestaurantDirectory() const { return restaurantDirectory; }

    FoodDeliveryApp() {
        srand(static_cast<unsigned int>(time(NULL))); 
        addRestaurant("Bombay Bytes", 4.8, "Indian", "BB");
        addMenuItem("BB", "Butter Chicken", 12.99, 25);
        addMenuItem("BB", "Garlic Naan", 3.50, 10);

        addRestaurant("Taco Town", 4.2, "Mexican", "TT");
        addMenuItem("TT", "Chili Taco", 9.50, 15);
        addMenuItem("TT", "Burrito Bowl", 14.00, 20);

        addRestaurant("The Wok", 3.9, "Chinese", "TW");
        addMenuItem("TW", "Noodles", 11.50, 25);

        std::cout << "App initialized with 3 restaurants and menus.\n";
        generateJsonOutput(); // Initial JSON output
    }
    
    void addRestaurant(const std::string& name, double rating, const std::string& cuisine, const std::string& id) {
        Restaurant r(name, rating, cuisine, id); 
        restaurantDirectory.push_back(r);
        topRatedRestaurants.push(r);
    }
    
    void addMenuItem(const std::string& menuId, const std::string& name, double price, int time) {
        MenuItem item(name, price, time);
        menus[menuId][name] = item; 
    }

    void placeOrder() {
        if (orderBasket.empty()) {
            std::cout << "[ORDER] Cannot place empty order.\n";
            return;
        }
        
        double total = 0.0;
        std::stack<OrderItem> tempStack = orderBasket;
        std::stringstream ss;
        
        while (!tempStack.empty()) {
            total += tempStack.top().price;
            ss << tempStack.top().itemName << ", ";
            tempStack.pop();
        }
        
        DeliveryOrder newOrder;
        newOrder.orderId = "ORD-" + std::to_string(nextOrderId++);
        newOrder.totalCost = total;
        
        std::string summary = ss.str();
        if (!summary.empty()) { summary.pop_back(); summary.pop_back(); }
        newOrder.summary = summary;
        
        // QUEUE: ENQUEUE
        deliveryQueue.push(newOrder);
        
        // Clear the basket
        while (!orderBasket.empty()) { orderBasket.pop(); }

        std::cout << "\n[SUCCESS] Order Placed! (std::queue::push)\n";
        generateJsonOutput(); 
    }

    void processNextDelivery() {
        if (deliveryQueue.empty()) {
            std::cout << "\n[DELIVERY] Queue is empty.\n";
            return;
        }
        
        // QUEUE: DEQUEUE
        DeliveryOrder deliveredOrder = deliveryQueue.front();
        deliveryQueue.pop();
        
        // LINKED LIST: ADD
        orderHistory.push_back(deliveredOrder);

        std::cout << "\n[DELIVERY] Processed Order: " << deliveredOrder.orderId << " (std::queue::pop & std::list::push_back)\n";
        generateJsonOutput();
    }
    
    // --- Console-only Methods ---
    void addItemToBasket(const std::string& restName, const std::string& menuId, const std::string& itemName) {
        if (menus.find(menuId) == menus.end() || menus[menuId].find(itemName) == menus[menuId].end()) {
            std::cout << "[ERROR] Item not found.\n";
            return;
        }
        
        MenuItem item = menus[menuId][itemName];
        // STACK: PUSH
        orderBasket.push(OrderItem(restName, item.itemName, item.price));
        std::cout << "[BASKET] ADDED: " << item.itemName << " (std::stack::push)\n";
        generateJsonOutput();
    }

    void removeLastItem() {
        if (orderBasket.empty()) {
            std::cout << "[BASKET] Basket is already empty.\n";
            return;
        }
        
        // STACK: POP
        OrderItem removed = orderBasket.top();
        orderBasket.pop();
        
        std::cout << "[BASKET] REMOVED: " << removed.itemName << " (std::stack::pop)\n";
        generateJsonOutput();
    }

    void viewOrderHistory() const {
        std::cout << "\n--- [CONSOLE] Order History (std::list) ---\n";
        if (orderHistory.empty()) {
            std::cout << "No completed orders.\n";
            return;
        }
        for (const auto& order : orderHistory) {
            std::cout << "ID: " << order.orderId << " | Total: $" << order.totalCost << "\n";
        }
    }
};

// --- 3. FRONTEND SIMULATION (MAIN FUNCTION) ---

void displayConsoleMenu() {
    std::cout << "\n==========================================\n";
    std::cout << "|        C++ BACKEND (CUSTOMER)       |\n";
    std::cout << "==========================================\n";
    std::cout << "Note: View http://localhost:8080/dashboard.html to see the live dashboard!\n";
    std::cout << "--- Basket (Stack) ---\n";
    std::cout << "1. Add 'Butter Chicken' (Stack PUSH)\n";
    std::cout << "2. Add 'Chili Taco' (Stack PUSH)\n";
    std::cout << "3. Remove Last Item (Stack POP)\n";
    std::cout << "\n--- Order (Queue & List) ---\n";
    std::cout << "4. **PLACE ORDER** (Queue ENQUEUE)\n";
    std::cout << "5. **PROCESS NEXT DELIVERY** (Queue DEQUEUE)\n";
    std::cout << "\n--- View (Console) ---\n";
    std::cout << "6. View Order History (Console)\n";
    std::cout << "7. Quit\n";
    std::cout << "------------------------------------------\n";
    std::cout << "Enter choice: ";
}

int main() {
    FoodDeliveryApp app;
    int choice;

    do {
        displayConsoleMenu();
        
        if (!(std::cin >> choice)) {
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        
        switch (choice) {
            case 1: app.addItemToBasket("Bombay Bytes", "BB", "Butter Chicken"); break;
            case 2: app.addItemToBasket("Taco Town", "TT", "Chili Taco"); break;
            case 3: app.removeLastItem(); break;
            case 4: app.placeOrder(); break;
            case 5: app.processNextDelivery(); break;
            case 6: app.viewOrderHistory(); break;
            case 7: std::cout << "Exiting simulator. Goodbye!\n"; break;
            default: std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 7);

    return 0;
}
