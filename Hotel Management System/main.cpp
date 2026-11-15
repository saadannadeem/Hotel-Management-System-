#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <limits>
#include <cctype>
using namespace std;

// Room and guest limits
const int max_rooms = 20;
const int max_guests = 100;
const int max_users = 50;

// Room arrays
int room_no[max_rooms] = {
    101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
    111, 112, 113, 114, 115, 116, 117, 118, 119, 120
};
string room_type[max_rooms];
float room_price[max_rooms];
string room_view[max_rooms];
string room_amenities[max_rooms];
bool is_vip[max_rooms];
int room_state[max_rooms];

// Guest arrays
int guest_booking_id[max_guests];
string guest_name[max_guests];
string guest_phone_no[max_guests];
int guest_room_no[max_guests];
int guest_nights[max_guests];
int extra_service_cost[max_guests];
bool is_guest_vip[max_guests];
string guest_status[max_guests];
string guest_username[max_guests];

// User arrays
string username[max_users];
string user_password[max_users];
string user_role[max_users];

// Other variables
int guest_count = 0;
int user_count = 0;
int next_booking_id = 10001;
float total_revenue = 0;

void save_users();
void view_rooms();
void book_room_menu(string current_user);
void cancel_booking_menu(string current_user, bool is_staff);
void search_guests();
void check_in_out();
int find_active_guest(int r_no);
void print_guest(int index);
string add_service(int r_no, string type);
string mark_vip(int r_no);
void my_bookings(string current_user);
void report();


string to_file(string text)
{
    if (text == "") return "-";
    for (char& c : text)
    {
        if (c == ' ') c = '_';
    }
    return text;
}

string from_file(string text)
{
    if (text == "-") return "";
    for (char& c : text)
    {
        if (c == '_') c = ' ';
    }
    return text;
}

string to_lower(string text)
{
    for (char& c : text)
    {
        c = tolower(c);
    }
    return text;
}

void save_data()
{
    ofstream room_file("rooms.txt");
    for (int i = 0; i < max_rooms; i++)
    {
        room_file << room_no[i] << " " << room_type[i] << " " << room_price[i] << " " << room_state[i] << " " << is_vip[i] << "\n";
    }
    room_file.close();

    ofstream guest_file("guest.txt");
    guest_file << next_booking_id << " " << total_revenue << "\n";
    for (int i = 0; i < guest_count; i++)
    {
        guest_file << guest_booking_id[i] << " "
            << to_file(guest_name[i]) << " "
            << to_file(guest_phone_no[i]) << " "
            << guest_room_no[i] << " "
            << guest_nights[i] << " "
            << extra_service_cost[i] << " "
            << is_guest_vip[i] << " "
            << to_file(guest_status[i]) << " "      // FIX: "Checked Out" has a space
            << to_file(guest_username[i]) << "\n";
    }
    guest_file.close();
}

bool load_data()
{
    guest_count = 0;
    ifstream room_file("rooms.txt");
    if (!room_file) return false;

    int i = 0;
    while (i < max_rooms && room_file >> room_no[i] >> room_type[i] >> room_price[i] >> room_state[i] >> is_vip[i])
    {
        i++;
    }
    room_file.close();

    ifstream guest_file("guest.txt");
    if (guest_file)
    {
        guest_file >> next_booking_id >> total_revenue;
        string temp_name, temp_phone, temp_status, temp_user;
        while (guest_count < max_guests &&
            guest_file >> guest_booking_id[guest_count] >>
            temp_name >> temp_phone >>
            guest_room_no[guest_count] >>
            guest_nights[guest_count] >>
            extra_service_cost[guest_count] >>
            is_guest_vip[guest_count] >>
            temp_status >>
            temp_user)
        {
            guest_name[guest_count] = from_file(temp_name);
            guest_phone_no[guest_count] = from_file(temp_phone);
            guest_status[guest_count] = from_file(temp_status);
            guest_username[guest_count] = from_file(temp_user);
            guest_count++;
        }
        guest_file.close();
    }
    return true;
}

void print_guest(int g)
{
    cout << "Booking ID: " << guest_booking_id[g] << endl;
    cout << "Guest Name: " << guest_name[g] << endl;
    cout << "Phone Number: " << guest_phone_no[g] << endl;
    cout << "Room Number: " << guest_room_no[g] << endl;
    cout << "Nights: " << guest_nights[g] << endl;
    cout << "Status: " << guest_status[g] << endl;
    if (is_guest_vip[g] == true) cout << "VIP Guest" << endl;
    if (guest_status[g] == "Reserved" || guest_status[g] == "Checked-in" || guest_status[g] == "Checked In")
    {
        cout << "Services: $" << fixed << setprecision(2) << extra_service_cost[g] << endl;
    }
}

void search_guest()
{
    string query;
    cout << "Enter name, booking ID or room number: ";
    cin >> query;

    int found = 0;
    for (int i = 0; i < guest_count; i++)
    {
        bool name_match = to_lower(guest_name[i]) == to_lower(query);
        bool id_match = to_string(guest_booking_id[i]) == query;
        bool room_match = to_string(guest_room_no[i]) == query;
        if (name_match || id_match || room_match)
        {
            print_guest(i);
            found++;
        }
    }
    if (found == 0) cout << "No guest found." << endl;
}

void save_receipt(int g)
{
    ofstream receipt_file("receipts.txt", ios::app);
    receipt_file << "Booking ID: " << guest_booking_id[g] << endl;
    receipt_file << "Guest Name: " << guest_name[g] << endl;
    receipt_file << "Room Number: " << guest_room_no[g] << endl;
    receipt_file << "Nights: " << guest_nights[g] << endl;
    receipt_file.close();
}


void init_rooms()
{
    for (int i = 0; i < max_rooms; i++)
    {
        room_no[i] = 101 + i;
        if (room_no[i] < 108)
        {
            room_type[i] = "Single";
            room_price[i] = 50;
            room_amenities[i] = "WiFi_AC";
        }
        else if (room_no[i] < 116)
        {
            room_type[i] = "Double";
            room_price[i] = 100;
            room_amenities[i] = "WiFi_AC_TV";
        }
        else
        {
            room_type[i] = "Suite";
            room_price[i] = 150;
            room_amenities[i] = "WiFi_AC_TV_Minibar";
        }

        if (i % 3 == 0) room_view[i] = "Sea";
        else if (i % 3 == 1) room_view[i] = "Garden";
        else room_view[i] = "City";

        room_state[i] = 0;
        is_vip[i] = false;
    }
}

void display_rooms()
{
    cout << "Room" << "\t" << "Type" << "\t" << "Price" << "\t"
        << "Status           " << "View    " << "Amenities" << endl;
    for (int i = 0; i < max_rooms; i++)
    {
        string status;
        if (room_state[i] == 0) status = "Free";
        else if (room_state[i] == 1) status = "Reserved";
        else status = "Checked In";

        if (is_vip[i] == 1) status += " (VIP)";
        while (status.length() < 17) status += " ";

        string view = room_view[i];
        while (view.length() < 8) view += " ";

        cout << room_no[i] << "\t" << room_type[i] << "\t" << "$" << room_price[i] << "\t"
            << status << view << room_amenities[i] << endl;
    }
}

int find_room(int r_no)
{
    for (int i = 0; i < max_rooms; i++)
    {
        if (room_no[i] == r_no) return i;
    }
    return -1;
}

int find_active_guest(int r_no)
{
    for (int g = 0; g < guest_count; g++)
    {
        if (guest_room_no[g] == r_no && (guest_status[g] == "Reserved" || guest_status[g] == "Checked In" || guest_status[g] == "Checked-in"))
        {
            return g;
        }
    }
    return -1;
}

string book_room_core(int r_no, string name, string phone, int nights, string current_user)
{
    int r = find_room(r_no);
    if (r == -1) return "Room does not exist";
    if (room_state[r] != 0) return "Room is not free";
    if (name == "") return "Name cannot be empty";
    if (guest_count == max_guests) return "Guest list is full";

    int g = guest_count;
    guest_booking_id[g] = next_booking_id++;
    guest_name[g] = name;
    guest_phone_no[g] = phone;
    guest_room_no[g] = r_no;
    guest_nights[g] = nights;
    extra_service_cost[g] = 0;
    is_guest_vip[g] = false;
    guest_status[g] = "Reserved";
    guest_username[g] = current_user;
    guest_count++;
    room_state[r] = 1;
    return "OK";
}

float get_subtotal(int g)
{
    int r = find_room(guest_room_no[g]);
    if (r == -1) return 0;
    return guest_nights[g] * room_price[r];
}

float get_discount(int g)
{
    float rate = 0;
    if (guest_nights[g] >= 5) rate += 0.1;
    if (is_guest_vip[g] == true) rate += 0.10;
    return rate * get_subtotal(g);
}

float get_tax(int g)
{
    float base = get_subtotal(g) - get_discount(g) + extra_service_cost[g];
    return base * 0.1;
}

float get_total(int g)
{
    float before_tax = get_subtotal(g) - get_discount(g) + extra_service_cost[g];
    return before_tax + get_tax(g);
}

void print_bill(int g)
{
    float subtotal = get_subtotal(g), discount = get_discount(g), tax = get_tax(g), total = get_total(g);
    cout << "----------Hotel LA Vida Bill----------" << endl;
    cout << "Guest:" << "\t\t" << guest_name[g] << endl;
    cout << "Room:" << "\t\t" << guest_room_no[g] << endl;
    cout << "Subtotal:" << "\t" << "$" << subtotal << endl;
    cout << "Discount:" << "\t" << "$" << discount << endl;
    cout << "Service:" << "\t" << "$" << extra_service_cost[g] << endl;
    cout << "Tax:" << "\t\t" << "$" << tax << endl;
    cout << "Total:" << "\t\t" << "$" << total << endl;
}

string check_in(int r_no)
{
    int r = find_room(r_no);
    int g = find_active_guest(r_no);
    if (g == -1) return "No reservation for this room";
    guest_status[g] = "Checked-in";
    room_state[r] = 2;
    is_vip[r] = is_guest_vip[g];
    return "OK";
}

string check_out(int r_no)
{
    int r = find_room(r_no);
    int g = find_active_guest(r_no);
    if (g == -1) return "No Checked In guest in this room";
    print_bill(g);
    save_receipt(g);
    total_revenue += get_total(g);
    guest_status[g] = "Checked Out";
    room_state[r] = 0;
    is_vip[r] = false;
    return "OK";
}

string cancel_booking_core(int r_no, string current_user, bool is_staff)
{
    int r = find_room(r_no);
    int g = find_active_guest(r_no);
    if (g == -1) return "No reservation for this room";
    if (guest_username[g] != current_user && !is_staff) return "This is not your booking";
    guest_status[g] = "Cancelled";
    room_state[r] = 0;
    return "OK";
}

// Modify booking: change number of nights (from the 4th member's staff panel)
string modify_nights(int r_no, int nights)
{
    int g = find_active_guest(r_no);
    if (g == -1) return "No active booking for this room";
    if (nights <= 0) return "Enter valid nights";
    guest_nights[g] = nights;
    return "OK";
}

// Modify booking: move the guest to another free room
string change_room(int r_no, int new_room)
{
    int g = find_active_guest(r_no);
    if (g == -1) return "No active booking for this room";

    int old_r = find_room(r_no);
    int new_r = find_room(new_room);
    if (new_r == -1 || room_state[new_r] != 0) return "That room is not free";

    room_state[new_r] = room_state[old_r];
    room_state[old_r] = 0;
    is_vip[new_r] = is_vip[old_r];
    is_vip[old_r] = false;
    guest_room_no[g] = new_room;
    return "OK";
}


// ==========================================
// Bridge / Wrapper Functions for 3rd Member Menu
// ==========================================
void view_rooms()
{
    display_rooms();
}

void book_room(string current_user)
{
    int r_no;
    cout << "Enter room number: ";
    cin >> r_no;
    string name, phone;
    cout << "Enter guest name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Enter phone number: ";
    getline(cin, phone);
    int nights;
    cout << "Enter number of nights: ";
    cin >> nights;

    string res = book_room_core(r_no, name, phone, nights, current_user);
    cout << res << endl;
}

void cancel_booking(string current_user, bool is_staff)
{
    int r_no;
    cout << "Enter room number to cancel: ";
    cin >> r_no;
    string res = cancel_booking_core(r_no, current_user, is_staff);
    cout << res << endl;
}

void search_guests()
{
    search_guest();
}

void check_in_out()
{
    int choice;
    cout << "1. Check-in\n2. Check-out\nChoice: ";
    cin >> choice;
    int r_no;
    cout << "Enter room number: ";
    cin >> r_no;

    if (choice == 1)
    {
        cout << check_in(r_no) << endl;
    }
    else
    {
        cout << check_out(r_no) << endl;
    }
}


int read_int(string prompt, int low, int high)
{
    int x;
    cout << prompt;
    while (true)
    {
        cin >> x;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again: ";
        }
        else if (x < low || x > high)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again: ";
        }
        else
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return x;
        }
    }
}

string readline(string prompt)
{
    string line;
    cout << prompt;
    getline(cin, line);
    return line;
}

void load_users()
{
    string name, pass, role;
    ifstream file("users.txt");
    while (user_count < max_users && file >> name >> pass >> role)
    {
        username[user_count] = name;
        user_password[user_count] = pass;
        user_role[user_count] = role;
        user_count++;
    }
    file.close();

    if (user_count == 0)
    {
        username[0] = "admin";
        user_password[0] = "1234";
        user_role[0] = "admin";

        username[1] = "reception";
        user_password[1] = "1234";
        user_role[1] = "receptionist";

        user_count = 2;
        save_users();
    }
}

void save_users()
{
    ofstream file("users.txt");
    int i = 0;
    while (i < user_count)
    {
        file << username[i] << " " << user_password[i] << " " << user_role[i] << endl;
        i++;
    }
    file.close();
}

int login()
{
    string userN, pass;
    cout << "Enter user name: ";
    cin >> userN;
    cout << "Enter password: ";
    cin >> pass;

    int i = 0;
    while (i < user_count)
    {
        if (userN == username[i] && pass == user_password[i])
        {
            return i;
        }
        i++;
    }
    return -1;
}

void register_customer()
{
    string userN, pass;
    bool check = false;
    int i = 0;

    cout << "Enter your user name: ";
    cin >> userN;
    cout << "Enter your password: ";
    cin >> pass;

    if (user_count >= max_users)
    {
        cout << "Capacity is full" << endl;
        return;
    }
    if (userN.empty() || pass.empty())
    {
        cout << "Username and password cannot be empty." << endl;
        return;
    }
    if (userN.find(' ') != string::npos || pass.find(' ') != string::npos)
    {
        cout << "Spaces are not allowed." << endl;
        return;
    }

    while (i < user_count)
    {
        if (userN == username[i])
        {
            check = true;
            cout << "Username already exists." << endl;
            break;
        }
        i++;
    }

    if (check == false)
    {
        username[user_count] = userN;
        user_password[user_count] = pass;
        user_role[user_count] = "customer";
        user_count++;

        save_users();
        cout << "Registration successful" << endl;
    }
}

void customer_menu(string current_user)
{
    int choice;
    do
    {
        cout << "\nEnter your choice:"
            << "\n1. View rooms"
            << "\n2. Book a room for self"
            << "\n3. My bookings"
            << "\n4. Cancel own booking"
            << "\n0. Logout";

        choice = read_int("\nChoice: ", 0, 4);

        if (choice == 1)
        {
            view_rooms();
            save_data();
        }
        else if (choice == 2)
        {
            book_room(current_user);
            save_data();
        }
        else if (choice == 3)
        {
            my_bookings(current_user);
            save_data();
        }
        else if (choice == 4)
        {
            cancel_booking(current_user, false);
            save_data();
        }

    } while (choice != 0);
}

void receptionist_menu()
{
    int choice;
    int r_no;
    string type;

    do
    {
        cout << "\nReceptionist Menu"
            << "\n1. View rooms"
            << "\n2. Book room for any guest"
            << "\n3. Search guests"
            << "\n4. Check-in/check-out"
            << "\n5. Add service"
            << "\n6. Mark VIP"
            << "\n7. Cancel any booking"
            << "\n0. Logout";

        choice = read_int("\nChoice: ", 0, 7);

        if (choice == 1)
        {
            view_rooms();
            save_data();
        }
        else if (choice == 2)
        {
            book_room("-");
            save_data();
        }
        else if (choice == 3)
        {
            search_guests();
            save_data();
        }
        else if (choice == 4)
        {
            check_in_out();
            save_data();
        }
        else if (choice == 5)
        {
            cout << "Enter room number: ";
            cin >> r_no;
            cout << "Enter service (meals/laundry/spa): ";
            cin >> type;
            cout << add_service(r_no, type) << endl;
            save_data();
        }
        else if (choice == 6)
        {
            cout << "Enter room number: ";
            cin >> r_no;
            cout << mark_vip(r_no) << endl;
            save_data();
        }
        else if (choice == 7)
        {
            cancel_booking("-", true);
            save_data();
        }

    } while (choice != 0);
}

void admin_menu()
{
    int choice;
    int r_no;
    string type;

    do
    {
        cout << "\nAdmin Menu"
            << "\n1. View rooms"
            << "\n2. Book room for any guest"
            << "\n3. Search guests"
            << "\n4. Check-in/check-out"
            << "\n5. Add service"
            << "\n6. Mark VIP"
            << "\n7. Cancel any booking"
            << "\n8. Reports"
            << "\n0. Logout";

        choice = read_int("\nChoice: ", 0, 8);

        if (choice == 1)
        {
            view_rooms();
            save_data();
        }
        else if (choice == 2)
        {
            book_room("-");
            save_data();
        }
        else if (choice == 3)
        {
            search_guests();
            save_data();
        }
        else if (choice == 4)
        {
            check_in_out();
            save_data();
        }
        else if (choice == 5)
        {
            cout << "Enter room number: ";
            cin >> r_no;
            cout << "Enter service (meals/laundry/spa): ";
            cin >> type;
            cout << add_service(r_no, type) << endl;
            save_data();
        }
        else if (choice == 6)
        {
            cout << "Enter room number: ";
            cin >> r_no;
            cout << mark_vip(r_no) << endl;
            save_data();
        }
        else if (choice == 7)
        {
            cancel_booking("-", true);
            save_data();
        }
        else if (choice == 8)
        {
            report();
            save_data();
        }

    } while (choice != 0);
}

string add_service(int r_no, string type)
{
    int index = find_active_guest(r_no);
    if (index == -1) return "Error: Guest not found";
    if (guest_status[index] != "Checked-in" && guest_status[index] != "Checked In") return "Guest is not checked in";

    if (type == "meals") extra_service_cost[index] += 15;
    else if (type == "laundry") extra_service_cost[index] += 10;
    else if (type == "spa") extra_service_cost[index] += 40;
    else return "Error: Invalid service";

    return "OK";
}

string mark_vip(int r_no)
{
    int index = find_active_guest(r_no);
    if (index == -1) return "Error";

    if (guest_status[index] == "Reserved")
    {
        is_guest_vip[index] = true;
        return "OK";
    }
    else if (guest_status[index] == "Checked-in" || guest_status[index] == "Checked In")
    {
        is_guest_vip[index] = true;
        for (int i = 0; i < max_rooms; i++)
        {
            if (room_no[i] == r_no)
            {
                is_vip[i] = true;
                break;
            }
        }
        return "OK";
    }
    return "Error";
}

void my_bookings(string current_user)
{
    bool check = false;
    int i = 0;
    while (i < guest_count)
    {
        if (guest_username[i] == current_user)
        {
            print_guest(i);
            check = true;
        }
        i++;
    }
    if (check == false) cout << "You have no bookings" << endl;
}

void report()
{
    int free_rooms = 0, reserved_rooms = 0, checkedin_rooms = 0;
    int i = 0;
    while (i < max_rooms)
    {
        if (room_state[i] == 0) free_rooms++;
        else if (room_state[i] == 1) reserved_rooms++;
        else if (room_state[i] == 2) checkedin_rooms++;
        i++;
    }
    cout << "\nFree rooms = " << free_rooms;
    cout << "\nReserved rooms = " << reserved_rooms;
    cout << "\nChecked-in rooms = " << checkedin_rooms;
    cout << "\nTotal revenue: $" << total_revenue << endl;
}


int console_main()
{
    cout << fixed << setprecision(2);

    init_rooms();
    if (!load_data())
    {
        save_data();
    }

    load_users();

    int choice;
    do
    {
        cout << "\n===== HOTEL ROOM BOOKING SYSTEM =====";
        cout << "\n1. Login";
        cout << "\n2. Register";
        cout << "\n0. Exit";

        choice = read_int("\nEnter your choice: ", 0, 2);

        if (choice == 1)
        {
            int i = login();
            if (i != -1)
            {
                if (user_role[i] == "customer")
                {
                    customer_menu(username[i]);
                }
                else if (user_role[i] == "receptionist")
                {
                    receptionist_menu();
                }
                else if (user_role[i] == "admin")
                {
                    admin_menu();
                }
            }
            else
            {
                cout << "\nInvalid username or password!" << endl;
            }
        }
        else if (choice == 2)
        {
            register_customer();
        }

    } while (choice != 0);

    save_data();

    return 0;
}


void run_gui();   

int main()
{
    init_rooms();           

    if (!load_data())       
        save_data();

    load_users();           

    run_gui();

    save_data();
    return 0;
}