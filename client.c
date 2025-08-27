#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdbool.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define MAX_USERNAME_LENGTH 100
#define MAX_PASSWORD_LENGTH 100
#define MAX_COURSE_NAME_LENGTH 100
#define MAX_RESPONSE_LENGTH 1024

// Global variables
int client_fd = -1;
bool is_logged_in = false;
char current_user_type[20] = "";
int current_user_id = -1;

// Function prototypes
void getParam(const char* str, char* result, int token_index);
void clearScreen();
void displayTitle(const char* title);
void displayError(const char* message);
void displaySuccess(const char* message);
void waitForEnter();
void Exit(int signal_num);
struct sockaddr_in configureServerAddress(const char *ip, int port);
void connectServer();
void sendRequest(const char* request, char* response);
void loginMenu();
void adminMenu();  
void studentMenu(); 
void facultyMenu(); 

void getParam(const char* str, char* result, int token_index) {
    char temp[BUFFER_SIZE];
    strcpy(temp, str);
    
    char* token = strtok(temp, " ");
    int i = 0;
    
    while (token != NULL && i < token_index) {
        token = strtok(NULL, " ");
        i++;
    }
    
    if (token != NULL) {
        strcpy(result, token);
    } else {
        result[0] = '\0';
    }
}

void clearScreen() {
    system("clear");
}

void displayTitle(const char* title) {
    printf("\n=== %s ===\n\n", title);
}

void displayError(const char* message) {
    printf("ERROR: %s (Press Enter to continue...)", message);
    getchar();
}

void displaySuccess(const char* message) {
    printf("SUCCESS: %s (Press Enter to continue...)", message);
    getchar();
}

void waitForEnter() {
    printf("\nPress Enter to continue...");
    getchar(); // Wait for Enter key
}

void Exit(int signal_num) {
    printf("\nExiting client application...\n");
    if (client_fd != -1) {
        close(client_fd);
    }
    exit(signal_num);
}

struct sockaddr_in configureServerAddress(const char *ip, int port) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported\n");
        exit(EXIT_FAILURE);
    }

    return serv_addr;
}

void connectServer() {
    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Socket creation error\n");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    struct sockaddr_in serv_addr = configureServerAddress(SERVER_IP, PORT);

    // Connect to server
    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed: Server might be offline\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to Academia Portal Server\n");
}

void sendRequest(const char* request, char* response) {
    // Initialize response buffer
    memset(response, 0, BUFFER_SIZE);
    
    // Send request to server
    if (send(client_fd, request, strlen(request), 0) < 0) {
        fprintf(stderr, "Failed to send request\n");
        strcpy(response, "ERROR");
        return;
    }
    
    // Receive response from server
    ssize_t bytesRead = read(client_fd, response, BUFFER_SIZE - 1);
    
    if (bytesRead < 0) {
        fprintf(stderr, "Failed to read response\n");
        strcpy(response, "ERROR");
        return;
    }
    
    // Ensure null termination
    response[bytesRead] = '\0';
}

void loginMenu() {
    while (!is_logged_in) {
        clearScreen();
        displayTitle("Academia Portal - Login");
        
        char username[MAX_USERNAME_LENGTH];
        char password[MAX_PASSWORD_LENGTH];
        char request[BUFFER_SIZE];
        char response[BUFFER_SIZE];
        
        printf("Username: ");
        scanf("%s", username);
        getchar(); // Clear input buffer
        
        printf("Password: ");
        scanf("%s", password);
        getchar(); // Clear input buffer
        
        // Send login request
        snprintf(request, BUFFER_SIZE, "LOGIN %s %s", username, password);
        sendRequest(request, response);
        
        // Process login response from server
        if (strcmp(response, "LOGIN_SUCCESS") == 0) {
            // Login successful - update user status
            is_logged_in = true;
            
            // Get user type from response (e.g., ADMIN, STUDENT, FACULTY)
            char user_type[20];
            getParam(response, user_type, 1);
            strcpy(current_user_type, user_type);
            
            // Get numeric user ID from response
            char user_id[10];
            getParam(response, user_id, 2);
            current_user_id = atoi(user_id);
            
            // Show success message to user
            displaySuccess("Login successful!");
        } 
        else {
            // Login failed - prepare error message for user
            char error_msg[BUFFER_SIZE] = "Login failed: ";
            
            // Extract specific error reason from server response
            char* error_details = strchr(response, ' '); //response is like ERROR <error_details>
            if (error_details != NULL) {
                // Append error details after the space
                strcat(error_msg, error_details + 1);
            } else {
                // No specific error provided
                strcat(error_msg, "Unknown error");
            }
            
            // Show error message to user
            displayError(error_msg);
        }
    }
    
    // Redirect to appropriate menu based on user type
    if (strcmp(current_user_type, "ADMIN") == 0) {
        adminMenu();
    } else if (strcmp(current_user_type, "STUDENT") == 0) {
        studentMenu();
    } else if (strcmp(current_user_type, "FACULTY") == 0) {
        facultyMenu();
    }
}

void adminMenu() {
    while (is_logged_in) {
        clearScreen();
        displayTitle("Admin Dashboard");
        
        printf("1. Add Student\n");
        printf("2. Add Faculty\n");
        printf("3. Activate/Deactivate Student\n");
        printf("4. Update Student/Faculty details\n");
        printf("5. View All Users\n");
        printf("6. View All Courses\n");
        printf("7. Logout\n");
        printf("8. Exit\n");
        
        printf("\nEnter your choice: ");
        
        int choice;
        scanf("%d", &choice);
        getchar(); // Clear input buffer
        
        char request[BUFFER_SIZE];
        char response[BUFFER_SIZE];
        
        switch (choice) {
            case 1: { // Add Student
                clearScreen();
                displayTitle("Add New Student");
                
                char username[MAX_USERNAME_LENGTH];
                char password[MAX_PASSWORD_LENGTH];
                
                printf("Enter username: ");
                scanf("%s", username);
                getchar(); // Clear input buffer
                
                printf("Enter password: ");
                scanf("%s", password);
                getchar(); // Clear input buffer
                
                snprintf(request, BUFFER_SIZE, "ADMIN %d ADD_STUDENT %s %s", current_user_id, username, password);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 2: { // Add Faculty
                clearScreen();
                displayTitle("Add New Faculty");
                
                char username[MAX_USERNAME_LENGTH];
                char password[MAX_PASSWORD_LENGTH];
                
                printf("Enter username: ");
                scanf("%s", username);
                getchar(); // Clear input buffer
                
                printf("Enter password: ");
                gscanf("%s", password);
                getchar(); // Clear input buffer
                
                snprintf(request, BUFFER_SIZE, "ADMIN %d ADD_FACULTY %s %s", current_user_id, username, password);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 3: { // Activate/Deactivate Student
                clearScreen();
                displayTitle("Activate/Deactivate Student");
                
                int studentId;
                
                printf("Enter student ID: ");
                scanf("%d", &studentId);
                getchar(); // Clear input buffer
                
                snprintf(request, BUFFER_SIZE, "ADMIN %d TOGGLE_STUDENT %d", current_user_id, studentId);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 4: { // Update User details
                clearScreen();
                displayTitle("Update User Details");
                
                int userId;
                char field[20];
                char value[MAX_PASSWORD_LENGTH];
                
                printf("Enter user ID: ");
                scanf("%d", &userId);
                getchar(); // Clear input buffer
                
                printf("What to update (username/password): ");
                scanf("%s", field);
                getchar(); // Clear input buffer
                
                if (strcmp(field, "password") == 0) {
                    printf("Enter new password: ");
                    scanf("%s", value);
                    getchar();
                } else {
                    printf("Enter new value: ");
                    scanf("%s", value);
                    getchar(); // Clear input buffer
                }
                
                snprintf(request, BUFFER_SIZE, "ADMIN %d UPDATE_USER %d %s %s", current_user_id, userId, field, value);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 5: { // View All Users
                clearScreen();
                displayTitle("All Users");
                
                snprintf(request, BUFFER_SIZE, "ADMIN %d VIEW_USERS", current_user_id);
                sendRequest(request, response);
                
                printf("%s\n", response);
                waitForEnter();
                break;
            }
            case 6: { // View All Courses
                clearScreen();
                displayTitle("All Courses");
                
                snprintf(request, BUFFER_SIZE, "ADMIN %d VIEW_COURSES", current_user_id);
                sendRequest(request, response);
                
                printf("%s\n", response);
                waitForEnter();
                break;
            }
            case 7: { // Logout
                is_logged_in = false;
                strcpy(current_user_type, "");
                current_user_id = -1;
                printf("Logged out successfully.\n");
                waitForEnter();
                return; // Return to login menu
            }
            case 8: { // Exit
                sendRequest("EXIT", response);
                Exit(0);
                break;
            }
            default:
                displayError("Invalid choice!");
        }
    }
}

void studentMenu() {
    while (is_logged_in) {
        clearScreen();
        displayTitle("Student Dashboard");
        
        printf("1. Enroll to new Courses\n");
        printf("2. Unenroll from already enrolled Courses\n");
        printf("3. View enrolled Courses\n");
        printf("4. View all available Courses\n");
        printf("5. Change Password\n");
        printf("6. Logout\n");
        printf("7. Exit\n");
        
        printf("\nEnter your choice: ");
        
        int choice;
        scanf("%d", &choice);
        getchar(); // Clear input buffer
        
        char request[BUFFER_SIZE];
        char response[BUFFER_SIZE];
        
        switch (choice) {
            case 1: { // Enroll to new course
                clearScreen();
                displayTitle("Enroll to New Course");
                
                // First, show available courses
                snprintf(request, BUFFER_SIZE, "STUDENT %d VIEW_COURSES", current_user_id);
                sendRequest(request, response);
                printf("%s\n", response);
                
                char courseCode[20];
                printf("\nEnter course code to enroll: ");
                scanf("%s", courseCode);
                getchar(); // Clear input buffer
                
                snprintf(request, BUFFER_SIZE, "STUDENT %d ENROLL %s", current_user_id, courseCode);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 2: { // Unenroll from course
                clearScreen();
                displayTitle("Unenroll from Course");
                
                // First, show enrolled courses
                snprintf(request, BUFFER_SIZE, "STUDENT %d VIEW_ENROLLED", current_user_id);
                sendRequest(request, response);
                printf("%s\n", response);
                
                char courseCode[20];
                printf("\nEnter course code to unenroll: ");
                scanf("%s", courseCode);
                getchar(); // Clear input buffer
                
                snprintf(request, BUFFER_SIZE, "STUDENT %d UNENROLL %s", current_user_id, courseCode);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 3: { // View enrolled courses
                clearScreen();
                displayTitle("Your Enrolled Courses");
                
                snprintf(request, BUFFER_SIZE, "STUDENT %d VIEW_ENROLLED", current_user_id);
                sendRequest(request, response);
                
                printf("%s\n", response);
                waitForEnter();
                break;
            }
            case 4: { // View all available courses
                clearScreen();
                displayTitle("All Available Courses");
                
                snprintf(request, BUFFER_SIZE, "STUDENT %d VIEW_COURSES", current_user_id);
                sendRequest(request, response);
                
                printf("%s\n", response);
                waitForEnter();
                break;
            }
            case 5: { // Change password
                clearScreen();
                displayTitle("Change Password");
                
                char oldPassword[MAX_PASSWORD_LENGTH];
                char newPassword[MAX_PASSWORD_LENGTH];
                
                printf("Enter current password: ");
                scanf("%s", oldPassword);
                getchar(); // Clear input buffer
                
                printf("\nEnter new password: ");
                scanf("%s", newPassword); 
                getchar(); // Clear input buffer

                snprintf(request, BUFFER_SIZE, "STUDENT %d CHANGE_PASSWORD %s %s", current_user_id, oldPassword, newPassword);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 6: { // Logout
                is_logged_in = false;
                strcpy(current_user_type, "");
                current_user_id = -1;
                printf("Logged out successfully.\n");
                waitForEnter();
                return; // Return to login menu
            }
            case 7: { // Exit
                sendRequest("EXIT", response);
                Exit(0);
                break;
            }
            default:
                displayError("Invalid choice!");
        }
    }
}

void facultyMenu() {
    while (is_logged_in) {
        clearScreen();
        displayTitle("Faculty Dashboard");
        
        printf("1. Add new Course\n");
        printf("2. Remove offered Course\n");
        printf("3. View enrollments in Courses\n");
        printf("4. View your Courses\n");
        printf("5. Change Password\n");
        printf("6. Logout\n");
        printf("7. Exit\n");
        
        printf("\nEnter your choice: ");
        
        int choice;
        scanf("%d", &choice);
        getchar(); // Clear input buffer
        
        char request[BUFFER_SIZE];
        char response[BUFFER_SIZE];
        
        switch (choice) {
            case 1: { // Add new course
                clearScreen();
                displayTitle("Add New Course");
                
                char courseCode[20];
                char courseName[MAX_COURSE_NAME_LENGTH];
                int seats;
                
                printf("Enter course code: ");
                scanf("%s", courseCode);
                getchar(); // Clear input buffer
                
                printf("Enter total seats: ");
                scanf("%d", &seats);
                getchar(); // Clear input buffer
                
                printf("Enter course name: ");
                fgets(courseName, MAX_COURSE_NAME_LENGTH, stdin);
                // Remove trailing newline
                size_t len = strlen(courseName);
                if (len > 0 && courseName[len-1] == '\n') {
                    courseName[len-1] = '\0';
                }
                
                snprintf(request, BUFFER_SIZE, "FACULTY %d ADD_COURSE %s %d %s", current_user_id, courseCode, seats, courseName);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 2: { // Remove offered course
                clearScreen();
                displayTitle("Remove Course");
                
                // First, show faculty's courses
                snprintf(request, BUFFER_SIZE, "FACULTY %d VIEW_COURSES", current_user_id);
                sendRequest(request, response);
                printf("%s\n", response);
                
                char courseCode[20];
                printf("\nEnter course code to remove: ");
                scanf("%s", courseCode);
                getchar(); // Clear input buffer
                
                snprintf(request, BUFFER_SIZE, "FACULTY %d REMOVE_COURSE %s", current_user_id, courseCode);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 3: { // View enrollments in courses
                clearScreen();
                displayTitle("Course Enrollments");
                
                snprintf(request, BUFFER_SIZE, "FACULTY %d VIEW_ENROLLMENTS", current_user_id);
                sendRequest(request, response);
                
                printf("%s\n", response);
                waitForEnter();
                break;
            }
            case 4: { // View faculty's courses
                clearScreen();
                displayTitle("Your Courses");
                
                snprintf(request, BUFFER_SIZE, "FACULTY %d VIEW_COURSES", current_user_id);
                sendRequest(request, response);
                
                printf("%s\n", response);
                waitForEnter();
                break;
            }
            case 5: { // Change password
                clearScreen();
                displayTitle("Change Password");
                
                char oldPassword[MAX_PASSWORD_LENGTH];
                char newPassword[MAX_PASSWORD_LENGTH];
                
                printf("Enter current password: ");
                scanf("%s", oldPassword);
                getchar(); // Clear input buffer
                
                printf("\nEnter new password: ");
                scanf("%s", newPassword);
                getchar(); // Clear input buffer

                snprintf(request, BUFFER_SIZE, "FACULTY %d CHANGE_PASSWORD %s %s", current_user_id, oldPassword, newPassword);
                sendRequest(request, response);
                
                displaySuccess(response);
                break;
            }
            case 6: { // Logout
                is_logged_in = false;
                strcpy(current_user_type, "");
                current_user_id = -1;
                printf("Logged out successfully.\n");
                waitForEnter();
                return; // Return to login menu
            }
            case 7: { // Exit
                sendRequest("EXIT", response);
                Exit(0);
                break;
            }
            default:
                displayError("Invalid choice!");
        }
    }
}

int main() {
    signal(SIGINT, Exit);
    connectServer();
    
    // Start with login menu
    while (true) {
        loginMenu();
    }
    
    return 0;
}