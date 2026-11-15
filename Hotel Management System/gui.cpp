#define _CRT_SECURE_NO_WARNINGS
#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFML/Audio.hpp>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <optional>
#include <cstdint>
using namespace std;


//Array Sizes
const int max_rooms = 20;
const int max_guests = 100;
const int max_users = 50;


//External data 
extern int room_no[max_rooms];
extern string room_type[max_rooms];
extern float room_price[max_rooms];
extern string room_view[max_rooms];
extern string room_amenities[max_rooms];
extern bool is_vip[max_rooms];
extern int room_state[max_rooms];

extern int guest_booking_id[max_guests];
extern string guest_name[max_guests];
extern string guest_phone_no[max_guests];
extern int guest_room_no[max_guests];
extern int guest_nights[max_guests];
extern int extra_service_cost[max_guests];
extern bool is_guest_vip[max_guests];
extern string guest_status[max_guests];
extern string guest_username[max_guests];
extern int guest_count;

extern string username[max_users];
extern string user_password[max_users];
extern string user_role[max_users];
extern int user_count;
extern float total_revenue;


//External functions 
void init_rooms();
bool load_data();
void save_data();
void load_users();
void save_users();
string to_lower(string text);
int find_room(int r_no);
int find_active_guest(int r_no);
string book_room_core(int r_no, string name, string phone, int nights, string current_user);
string cancel_booking_core(int r_no, string current_user, bool is_staff);
string check_in(int r_no);
string check_out(int r_no);
string add_service(int r_no, string type);
string mark_vip(int r_no);
string modify_nights(int r_no, int nights);
string change_room(int r_no, int new_room);
float get_subtotal(int g);
float get_discount(int g);
float get_tax(int g);
float get_total(int g);


//Screens
enum Screen { WELCOME, LOGIN, REGISTER, ROOMS, BOOKING, SEARCH, DETAILS, MYBOOKINGS, BILL, REPORT, MODIFY, RECOMMEND, RECEIPT };

//Shared Globals
Screen screen = WELCOME;
string current_user = "";
string current_role = "";
int selected_room = 0;
string message = "";
sf::Font font;
string fields[8];
int active_field = 0;
bool vip_checked = false;


float room_anim[max_rooms];          
sf::SoundBuffer book_buffer;
std::optional<sf::Sound> book_sound;

//Login / Register / Search inputs
string input_username = "";
string input_password = "";
string search_query = "";
string modify_input = "";
string recommend_input = "";
bool username_active = true;

//Mouse And Keyboard Info
bool mouse_clicked = false;
bool was_pressed = false;
sf::Vector2f click_pos;
bool char_typed = false;
unsigned int typed_char = 0;


//Helper Functions
bool draw_field(sf::RenderWindow& window, float x, float y, float w, string text, bool active)
{
	sf::RectangleShape box({ w, 40 });
	box.setPosition({ x, y });
	box.setFillColor(sf::Color::White);
	box.setOutlineThickness(3);

	if (active == true)
		box.setOutlineColor(sf::Color::Blue);
	else
		box.setOutlineColor(sf::Color(150, 150, 150));
	window.draw(box);

	sf::Text shown(font, text, 22);
	shown.setFillColor(sf::Color::Black);
	shown.setPosition({ x + 8, y + 5 });
	window.draw(shown);

	return mouse_clicked && box.getGlobalBounds().contains(click_pos);
}

int to_number(string s)
{
	if (s.length() == 0)
		return -1;
	int result = 0;
	for (int i = 0; i < s.length(); i++)
	{
		if (s[i] < '0' || s[i] > '9')
			return -1;
		result = result * 10 + (s[i] - '0');
	}
	return result;
}

string money(float amount)
{
	stringstream ss;
	ss << fixed << setprecision(2) << amount;
	return "$" + ss.str();
}


void draw_message(sf::RenderWindow& window, float x, float y)
{
	sf::Text t(font, message, 24);
	t.setPosition({ x, y });
	if (message.find("successful") != string::npos || message == "Done")
		t.setFillColor(sf::Color::Green);
	else
		t.setFillColor(sf::Color::Red);
	window.draw(t);
}


void type_into(string& s, size_t max_length)
{
	if (!char_typed)
		return;
	if (typed_char == 8 && s.size() > 0)
		s.pop_back();
	else if (typed_char >= 32 && typed_char <= 125 && s.size() < max_length)
		s += (char)typed_char;
}

string try_booking()
{
	if (fields[0] == "")
		return "Name cannot be empty";

	int day1 = to_number(fields[2]);
	int month1 = to_number(fields[3]);
	int year1 = to_number(fields[4]);
	int day2 = to_number(fields[5]);
	int month2 = to_number(fields[6]);
	int year2 = to_number(fields[7]);

	if (day1 == -1 || month1 == -1 || year1 == -1 || day2 == -1 || month2 == -1 || year2 == -1)
		return "Dates must be numbers";
	if (month1 < 1 || month1 > 12 || month2 < 1 || month2 > 12)
		return "Invalid month";
	if (year1 < 2000 || year1 > 2100 || year2 < 2000 || year2 > 2100)
		return "Invalid year";

	tm checkin;
	checkin.tm_sec = 0;
	checkin.tm_min = 0;
	checkin.tm_hour = 12;
	checkin.tm_mday = day1;
	checkin.tm_mon = month1 - 1;
	checkin.tm_year = year1 - 1900;
	checkin.tm_isdst = -1;

	tm checkout;
	checkout.tm_sec = 0;
	checkout.tm_min = 0;
	checkout.tm_hour = 12;
	checkout.tm_mday = day2;
	checkout.tm_mon = month2 - 1;
	checkout.tm_year = year2 - 1900;
	checkout.tm_isdst = -1;

	time_t time1 = mktime(&checkin);
	time_t time2 = mktime(&checkout);

	if (checkin.tm_mday != day1 || checkout.tm_mday != day2)
		return "Invalid day";

	double seconds = difftime(time2, time1);
	int nights = (int)((seconds + 43200) / 86400);

	if (nights <= 0)
		return "Check-out must be after check-in";

	string result = book_room_core(selected_room, fields[0], fields[1], nights, current_user);

	if (result == "OK")
	{
		int g = guest_count - 1;
		is_guest_vip[g] = vip_checked;
		save_data();
	}
	return result;
}
void draw_text(sf::RenderWindow& window, string words, float x, float y, int size)
{
	sf::Text t(font, words, size);
	t.setPosition({ x, y });
	window.draw(t);
}
bool draw_button(sf::RenderWindow& window, float x, float y, float w, float h, string label)
{
	sf::RectangleShape box({ w, h });
	box.setPosition({ x, y });
	box.setFillColor(sf::Color(60, 60, 150));
	window.draw(box);

	sf::Text text(font, label, 24);
	text.setPosition({ x + 15, y + 10 });
	window.draw(text);

	return mouse_clicked && box.getGlobalBounds().contains(click_pos);
}


//Roles and accounts
bool role_allows(string action)
{
	if (current_role == "admin")
		return true;

	if (current_role == "receptionist")
		return action != "Report";

	if (current_role == "customer")
		return action == "Book" || action == "My Bookings" || action == "Cancel";

	return false;
}

bool login_user()
{
	for (int i = 0; i < user_count; i++)
	{
		if (username[i] == input_username && user_password[i] == input_password)
		{
			current_user = username[i];
			current_role = user_role[i];
			input_username = "";
			input_password = "";
			message = "";
			screen = ROOMS;
			return true;
		}
	}

	message = "Incorrect username or password";
	return false;
}

bool register_user()
{
	if (input_username == "" || input_password == "")
	{
		message = "Username and password cannot be empty";
		return false;
	}

	if (input_username.find(' ') != string::npos || input_password.find(' ') != string::npos)
	{
		message = "No spaces allowed";
		return false;
	}

	if (user_count >= max_users)
	{
		message = "User limit reached";
		return false;
	}

	for (int i = 0; i < user_count; i++)
	{
		if (username[i] == input_username)
		{
			message = "Username already exists";
			return false;
		}
	}

	username[user_count] = input_username;
	user_password[user_count] = input_password;
	user_role[user_count] = "customer";
	user_count++;

	save_users();

	input_username = "";
	input_password = "";
	message = "Registration successful. Login now";
	screen = LOGIN;
	return true;
}


//Screens
void draw_welcome_screen(sf::RenderWindow& window) {

	sf::Text title(font, "Hotel LA Vida", 60);
	title.setPosition({ 300, 150 });
	window.draw(title);

	if (draw_button(window, 400, 300, 200, 60, "Login"))
		screen = LOGIN;

}

void draw_login_screen(sf::RenderWindow& window) {
	draw_text(window, "HOTEL MANAGEMENT SYSTEM", 300, 60, 36);
	draw_text(window, "LOGIN", 450, 130, 30);

	draw_text(window, "Username", 350, 190, 22);
	if (draw_field(window, 350, 220, 300, input_username, username_active))
		username_active = true;

	draw_text(window, "Password", 350, 290, 22);
	if (draw_field(window, 350, 320, 300, string(input_password.length(), '*'), !username_active))
		username_active = false;

	if (username_active)
		type_into(input_username, 20);
	else
		type_into(input_password, 20);

	if (draw_button(window, 350, 400, 140, 50, "Login"))
		login_user();

	if (draw_button(window, 510, 400, 140, 50, "Register"))
	{
		screen = REGISTER;
		message = "";
		input_username = "";
		input_password = "";
		username_active = true;
	}

	draw_message(window, 350, 480);
}

void draw_register_screen(sf::RenderWindow& window) {
	draw_text(window, "CREATE ACCOUNT", 400, 100, 34);

	draw_text(window, "Username", 350, 190, 22);
	if (draw_field(window, 350, 220, 300, input_username, username_active))
		username_active = true;

	draw_text(window, "Password", 350, 290, 22);
	if (draw_field(window, 350, 320, 300, string(input_password.length(), '*'), !username_active))
		username_active = false;

	if (username_active)
		type_into(input_username, 20);
	else
		type_into(input_password, 20);

	if (draw_button(window, 350, 400, 140, 50, "Register"))
		register_user();

	if (draw_button(window, 510, 400, 140, 50, "Back"))
	{
		screen = LOGIN;
		message = "";
		input_username = "";
		input_password = "";
		username_active = true;
	}

	draw_message(window, 350, 480);
}

void draw_rooms_screen(sf::RenderWindow& window) {
	draw_message(window, 30, 40);
	for (int i = 0; i < 20; i++)
	{
		int row = i / 5;
		int col = i % 5;
		float x = 30 + col * 140;
		float y = 100 + row * 120;

		sf::RectangleShape box({ 120, 100 });
		box.setPosition({ x, y });

		sf::Color final_color;
		if (is_vip[i] == true)         final_color = sf::Color(255, 215, 0);
		else if (room_state[i] == 1)   final_color = sf::Color::Blue;
		else if (room_state[i] == 2)   final_color = sf::Color::Red;
		else                           final_color = sf::Color::Green;

		if (room_state[i] == 0)
			room_anim[i] = 0;                               
		else
			room_anim[i] += (1 - room_anim[i]) * 0.1f;      

		sf::Color green = sf::Color::Green;
		box.setFillColor(sf::Color(
			static_cast<std::uint8_t>(green.r + (final_color.r - green.r) * room_anim[i]),
			static_cast<std::uint8_t>(green.g + (final_color.g - green.g) * room_anim[i]),
			static_cast<std::uint8_t>(green.b + (final_color.b - green.b) * room_anim[i])));
		window.draw(box);

		sf::Text number(font, to_string(room_no[i]), 28);
		number.setPosition({ x + 30, y + 30 });
		window.draw(number);

		if (mouse_clicked && box.getGlobalBounds().contains(click_pos))
		{
			message = "";
			selected_room = room_no[i];
			if (room_state[i] == 0)
				screen = BOOKING;
			else
				screen = DETAILS;
		}
	}

	// side panel (user info + navigation)
	draw_text(window, "Welcome: " + current_user, 760, 100, 22);
	draw_text(window, "Role: " + current_role, 760, 135, 22);

	if (current_role == "customer")
	{
		if (draw_button(window, 760, 200, 210, 50, "My Bookings"))
		{
			message = "";
			screen = MYBOOKINGS;
		}
	}

	if (current_role == "receptionist" || current_role == "admin")
	{
		if (draw_button(window, 760, 270, 210, 50, "Search Guest"))
		{
			message = "";
			search_query = "";
			screen = SEARCH;
		}
	}

	if (role_allows("Report"))
	{
		if (draw_button(window, 760, 340, 210, 50, "Report"))
		{
			message = "";
			screen = REPORT;
		}
	}

	if (current_role == "customer")
	{
		if (draw_button(window, 760, 410, 210, 50, "Recommend"))
		{
			message = "";
			recommend_input = "";
			screen = RECOMMEND;
		}
	}

	if (draw_button(window, 760, 600, 210, 50, "Logout"))
	{
		current_user = "";
		current_role = "";
		message = "";
		username_active = true;
		screen = LOGIN;
	}
}

void draw_booking_screen(sf::RenderWindow& window) {
	draw_text(window, "Book Room " + to_string(selected_room), 100, 30, 40);

	draw_text(window, "Name", 100, 100, 22);
	if (draw_field(window, 250, 95, 300, fields[0], active_field == 0))
		active_field = 0;

	draw_text(window, "Phone", 100, 160, 22);
	if (draw_field(window, 250, 155, 300, fields[1], active_field == 1))
		active_field = 1;

	draw_text(window, "Day", 250, 210, 18);
	draw_text(window, "Month", 330, 210, 18);
	draw_text(window, "Year", 410, 210, 18);

	draw_text(window, "Check-in", 100, 240, 22);
	if (draw_field(window, 250, 235, 70, fields[2], active_field == 2))
		active_field = 2;
	if (draw_field(window, 330, 235, 70, fields[3], active_field == 3))
		active_field = 3;
	if (draw_field(window, 410, 235, 100, fields[4], active_field == 4))
		active_field = 4;

	draw_text(window, "Check-out", 100, 300, 22);
	if (draw_field(window, 250, 295, 70, fields[5], active_field == 5))
		active_field = 5;
	if (draw_field(window, 330, 295, 70, fields[6], active_field == 6))
		active_field = 6;
	if (draw_field(window, 410, 295, 100, fields[7], active_field == 7))
		active_field = 7;

	sf::RectangleShape check({ 30, 30 });
	check.setPosition({ 250, 370 });
	if (vip_checked == true)
		check.setFillColor(sf::Color::Green);
	else
		check.setFillColor(sf::Color::White);
	window.draw(check);

	if (mouse_clicked && check.getGlobalBounds().contains(click_pos))
	{
		if (vip_checked == true)
			vip_checked = false;
		else
			vip_checked = true;
	}
	draw_text(window, "VIP guest", 295, 370, 22);

	if (char_typed)
	{
		int max_length = 20;
		if (active_field == 1)
			max_length = 15;
		if (active_field >= 2 && active_field <= 7)
			max_length = 2;
		if (active_field == 4 || active_field == 7)
			max_length = 4;

		int length = fields[active_field].length();

		if (typed_char == 8 && length > 0)
			fields[active_field].pop_back();

		if (typed_char >= 32 && typed_char <= 125 && length < max_length)
			fields[active_field] += (char)typed_char;
	}

	draw_message(window, 100, 440);

	if (draw_button(window, 250, 500, 200, 60, "Book"))
	{
		message = try_booking();
		if (message == "OK")
		{
			for (int i = 0; i < 8; i++)
				fields[i] = "";
			vip_checked = false;
			active_field = 0;
			message = "Room booked successfully";
			if (book_sound)
				book_sound->play();
			screen = ROOMS;
		}
	}

	if (draw_button(window, 480, 500, 200, 60, "Back"))
	{
		message = "";
		screen = ROOMS;
	}
}

void draw_search_screen(sf::RenderWindow& window) {
	draw_text(window, "SEARCH GUEST", 100, 40, 40);
	draw_text(window, "Name, Booking ID or Room Number", 100, 105, 22);
	draw_field(window, 100, 140, 400, search_query, true);
	type_into(search_query, 25);

	float y = 210;
	int found = 0;

	for (int i = 0; i < guest_count; i++)
	{
		bool name_match =
			to_lower(guest_name[i]).find(to_lower(search_query)) != string::npos;
		bool id_match = to_string(guest_booking_id[i]).find(search_query) != string::npos;
		bool room_match = to_string(guest_room_no[i]).find(search_query) != string::npos;

		if (search_query != "" && (name_match || id_match || room_match))
		{
			if (y < 570)
			{
				draw_text(window, "ID: " + to_string(guest_booking_id[i]) +
					" | " + guest_name[i] +
					" | Room: " + to_string(guest_room_no[i]) +
					" | " + guest_status[i], 100, y, 22);
				y += 35;
			}
			found++;
		}
	}

	if (search_query != "" && found == 0)
		draw_text(window, "No guest found.", 100, y, 24);

	if (draw_button(window, 100, 620, 150, 50, "Back"))
	{
		message = "";
		screen = ROOMS;
	}
}


void show_result(string result)
{
	if (result == "OK")
		message = "Done";
	else
		message = result;
	save_data();
}

void draw_details_screen(sf::RenderWindow& window) {
	int r = find_room(selected_room);

	draw_text(window, "Room " + to_string(selected_room), 100, 80, 50);
	draw_text(window, "Type: " + room_type[r], 100, 180, 30);
	draw_text(window, "Price: $" + to_string((int)room_price[r]) + " per night", 100, 230, 30);
	draw_text(window, "View: " + room_view[r], 100, 280, 30);
	draw_text(window, "Amenities: " + room_amenities[r], 100, 330, 30);

	bool staff = (current_role == "receptionist" || current_role == "admin");

	if (draw_button(window, 100, 450, 200, 60, "Back"))
	{
		message = "";
		screen = ROOMS;
	}

	if (staff)
	{
		if (draw_button(window, 330, 450, 200, 60, "View Bill"))
			screen = BILL;

		if (room_state[r] == 1)
		{
			if (draw_button(window, 560, 450, 230, 60, "Cancel Booking"))
			{
				message = cancel_booking_core(selected_room, current_user, true);
				if (message == "OK")
				{
					save_data();
					message = "Booking cancelled successfully";
					screen = ROOMS;
				}
			}
		}

		if (draw_button(window, 560, 530, 230, 60, "Modify Booking"))
		{
			message = "";
			modify_input = "";
			screen = MODIFY;
		}

		// check in / check out depending on room state
		if (room_state[r] == 1)
		{
			if (draw_button(window, 100, 530, 200, 60, "Check In"))
				show_result(check_in(selected_room));
		}
		else if (room_state[r] == 2)
		{
			if (draw_button(window, 100, 530, 200, 60, "Check Out"))
				show_result(check_out(selected_room));
		}

		if (draw_button(window, 330, 530, 200, 60, "Mark VIP"))
			show_result(mark_vip(selected_room));

		if (room_state[r] == 2)
		{
			draw_text(window, "Add service:", 100, 615, 22);
			if (draw_button(window, 250, 605, 120, 50, "Meals"))
				show_result(add_service(selected_room, "meals"));
			if (draw_button(window, 390, 605, 150, 50, "Laundry"))
				show_result(add_service(selected_room, "laundry"));
			if (draw_button(window, 560, 605, 100, 50, "Spa"))
				show_result(add_service(selected_room, "spa"));
		}

		draw_message(window, 100, 395);
	}
}

void draw_mybookings_screen(sf::RenderWindow& window) {
	draw_text(window, "MY BOOKINGS", 100, 25, 40);

	float y = 90;

	for (int i = 0; i < guest_count; i++)
	{
		if (guest_username[i] == current_user && y < 540)
		{
			draw_text(window, "Booking ID: " + to_string(guest_booking_id[i]), 100, y, 22);
			draw_text(window, "Room: " + to_string(guest_room_no[i]), 100, y + 28, 22);
			draw_text(window, "Nights: " + to_string(guest_nights[i]), 100, y + 56, 22);
			draw_text(window, "Status: " + guest_status[i], 100, y + 84, 22);

			if (guest_status[i] == "Reserved" || guest_status[i] == "Checked In" || guest_status[i] == "Checked-in")
				draw_text(window, "Running Bill: " + money(get_total(i)), 400, y, 22);

			if (guest_status[i] == "Reserved")
			{
				if (draw_button(window, 700, y + 25, 150, 50, "Cancel"))
				{
					message = cancel_booking_core(guest_room_no[i], current_user, false);

					if (message == "OK")
					{
						save_data();
						message = "Booking cancelled successfully";
					}
				}
			}

			y += 130;
		}
	}

	if (draw_button(window, 100, 620, 150, 50, "Back"))
	{
		screen = ROOMS;
		message = "";
	}

	draw_message(window, 300, 630);
}

void draw_bill_screen(sf::RenderWindow& window) {
	int g = find_active_guest(selected_room);

	if (g == -1)
	{
		draw_text(window, "No guest in this room", 100, 100, 30);
	}
	else
	{
		int r = find_room(selected_room);

		draw_text(window, "Hotel LA Vida Bill", 100, 40, 40);
		draw_text(window, "Guest: " + guest_name[g], 100, 110, 26);
		draw_text(window, "Room: " + to_string(selected_room), 100, 150, 26);
		draw_text(window, "Nights: " + to_string(guest_nights[g]), 100, 190, 26);
		draw_text(window, "Price per night: " + money(room_price[r]), 100, 230, 26);
		draw_text(window, "Subtotal: " + money(get_subtotal(g)), 100, 270, 26);
		draw_text(window, "Discount: " + money(get_discount(g)), 100, 310, 26);
		draw_text(window, "Services: " + money(extra_service_cost[g]), 100, 350, 26);
		draw_text(window, "Tax: " + money(get_tax(g)), 100, 390, 26);
		draw_text(window, "Total: " + money(get_total(g)), 100, 440, 34);
	}

	if (draw_button(window, 100, 520, 200, 60, "Back"))
		screen = DETAILS;

	if (draw_button(window, 330, 520, 200, 60, "Receipt"))
		screen = RECEIPT;
}

void draw_report_screen(sf::RenderWindow& window) {
	int free_rooms = 0, reserved_rooms = 0, checkedin_rooms = 0;
	for (int i = 0; i < max_rooms; i++)
	{
		if (room_state[i] == 0) free_rooms++;
		else if (room_state[i] == 1) reserved_rooms++;
		else checkedin_rooms++;
	}

	draw_text(window, "REPORT", 100, 60, 50);
	draw_text(window, "Free rooms: " + to_string(free_rooms), 100, 170, 30);
	draw_text(window, "Reserved rooms: " + to_string(reserved_rooms), 100, 220, 30);
	draw_text(window, "Checked-in rooms: " + to_string(checkedin_rooms), 100, 270, 30);
	draw_text(window, "Total revenue: " + money(total_revenue), 100, 340, 30);

	if (draw_button(window, 100, 450, 200, 60, "Back"))
		screen = ROOMS;
}

void draw_modify_screen(sf::RenderWindow& window) {
	int g = find_active_guest(selected_room);

	draw_text(window, "Modify Booking - Room " + to_string(selected_room), 100, 40, 40);

	if (g == -1)
	{
		draw_text(window, "No active booking for this room", 100, 130, 28);
	}
	else
	{
		draw_text(window, "Guest: " + guest_name[g], 100, 120, 26);
		draw_text(window, "Current nights: " + to_string(guest_nights[g]), 100, 160, 26);

		draw_text(window, "New nights OR new room number", 100, 215, 22);
		draw_field(window, 100, 250, 300, modify_input, true);
		type_into(modify_input, 6);

		if (draw_button(window, 100, 320, 220, 60, "Change Nights"))
		{
			int n = to_number(modify_input);
			if (n <= 0)
				message = "Enter valid nights";
			else
			{
				message = modify_nights(selected_room, n);
				if (message == "OK")
				{
					save_data();
					message = "Nights updated successfully";
				}
			}
			modify_input = "";
		}

		if (draw_button(window, 350, 320, 220, 60, "Change Room"))
		{
			int nr = to_number(modify_input);
			message = change_room(selected_room, nr);
			if (message == "OK")
			{
				selected_room = nr;
				save_data();
				message = "Room changed successfully";
			}
			modify_input = "";
		}
	}

	draw_message(window, 100, 410);

	if (draw_button(window, 100, 600, 200, 60, "Back"))
	{
		int r = find_room(selected_room);
		screen = (r != -1 && room_state[r] != 0) ? DETAILS : ROOMS;
	}
}

void draw_recommend_screen(sf::RenderWindow& window) {
	draw_text(window, "ROOM RECOMMENDATION", 100, 30, 40);
	draw_text(window, "Budget per night ($)", 100, 100, 22);
	draw_field(window, 100, 135, 300, recommend_input, true);
	type_into(recommend_input, 6);

	int budget = to_number(recommend_input);
	float y = 200;
	int found = 0;

	if (budget >= 0)
	{
		for (int i = 0; i < max_rooms; i++)
		{
			if (room_state[i] == 0 && room_price[i] <= budget)
			{
				if (y < 590)
				{
					draw_text(window, "Room " + to_string(room_no[i]) +
						" | " + room_type[i] +
						" | $" + to_string((int)room_price[i]) +
						" | " + room_view[i] + " view" +
						" | " + room_amenities[i], 100, y, 22);
					y += 32;
				}
				found++;
			}
		}

		if (found == 0)
			draw_text(window, "No free rooms found within this budget.", 100, y, 24);
	}

	if (draw_button(window, 100, 620, 150, 50, "Back"))
	{
		message = "";
		screen = ROOMS;
	}
}

void receipt_line(sf::RenderWindow& window, string label, string amount, float y, int size)
{
	draw_text(window, label, 150, y, size);
	draw_text(window, amount, 700, y, size);
}

void draw_receipt_screen(sf::RenderWindow& window) {
	int g = find_active_guest(selected_room);

	if (g == -1)
	{
		draw_text(window, "No guest in this room", 100, 100, 30);
	}
	else
	{
		int r = find_room(selected_room);

		time_t now = time(0);
		tm* t = localtime(&now);
		char date[20];
		strftime(date, sizeof(date), "%d-%m-%Y", t);

		draw_text(window, "HOTEL LA VIDA", 340, 30, 44);
		draw_text(window, "INVOICE", 440, 85, 28);
		draw_text(window, "Invoice No: INV-" + to_string(guest_booking_id[g]), 150, 140, 22);
		draw_text(window, "Date: " + string(date), 700, 140, 22);
		draw_text(window, "Guest: " + guest_name[g], 150, 175, 22);
		draw_text(window, "Room: " + to_string(selected_room), 700, 175, 22);

		sf::RectangleShape line({ 700, 2 });
		line.setPosition({ 150, 220 });
		window.draw(line);

		receipt_line(window, "Room: " + to_string(guest_nights[g]) + " nights x " + money(room_price[r]), money(get_subtotal(g)), 240, 24);
		receipt_line(window, "Discount", "-" + money(get_discount(g)), 285, 24);
		receipt_line(window, "Services", money(extra_service_cost[g]), 330, 24);
		receipt_line(window, "Tax", money(get_tax(g)), 375, 24);

		line.setPosition({ 150, 430 });
		window.draw(line);
		receipt_line(window, "TOTAL", money(get_total(g)), 450, 32);
	}

	if (draw_button(window, 100, 600, 200, 60, "Back"))
		screen = BILL;
}

//Main GUI loop
void run_gui()
{
	sf::RenderWindow window(sf::VideoMode({ 1000,700 }), "LA Vida Hotel");
	window.setFramerateLimit(60);
	if (!font.openFromFile("arial.ttf"))
	{
		cout << "Could not load arial.ttf" << endl;
		return;
	}

	
	for (int i = 0; i < max_rooms; i++)
		room_anim[i] = (room_state[i] == 0) ? 0 : 1;

	
	if (book_buffer.loadFromFile("book.wav"))
		book_sound.emplace(book_buffer);

	while (window.isOpen())
	{
		char_typed = false;
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();

			if (auto text = event->getIf<sf::Event::TextEntered>())
			{
				char_typed = true;
				typed_char = text->unicode;
			}
		}
		bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		mouse_clicked = pressed && !was_pressed;
		was_pressed = pressed;
		click_pos = sf::Vector2f(sf::Mouse::getPosition(window));
		window.clear();
		switch (screen)
		{
		case WELCOME:    draw_welcome_screen(window);    break;
		case LOGIN:      draw_login_screen(window);      break;
		case REGISTER:   draw_register_screen(window);   break;
		case ROOMS:      draw_rooms_screen(window);      break;
		case BOOKING:    draw_booking_screen(window);    break;
		case SEARCH:     draw_search_screen(window);     break;
		case DETAILS:    draw_details_screen(window);    break;
		case MYBOOKINGS: draw_mybookings_screen(window); break;
		case BILL:       draw_bill_screen(window);       break;
		case REPORT:     draw_report_screen(window);     break;
		case MODIFY:     draw_modify_screen(window);     break;
		case RECOMMEND:  draw_recommend_screen(window);  break;
		case RECEIPT:    draw_receipt_screen(window);    break;
		}
		window.display();
	}
}