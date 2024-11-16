#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

class User;
class Content
{
protected:
    string text;
    int likes;

public:
    Content(string txt) : text(txt), likes(0) {}
    Content(string txt, int like) : text(txt), likes(like) {}

    virtual void display() const = 0; // Polymorphic display method

    void like() { likes++; }

    string getText() const { return text; }
    int getLikes() const { return likes; }

    virtual void saveToFile(ofstream &file) const = 0;
};

class Post : public Content
{
private:
    User *author;

public:
    Post(string txt, User *auth) : Content(txt), author(auth) {}
    Post(string txt, int like, User *auth) : Content(txt, like), author(auth) {}

    void display() const;

    void saveToFile(ofstream &file) const;

    static vector<Post *> loadFromFile(vector<User *> &users);
};

class User
{
private:
    string username;
    string password;
    vector<User *> following;
    vector<User *> followers;
    vector<Post *> posts;

public:
    User(string name) : username(name) {}

    string getUsername() const { return username; }

    void follow(User *user)
    {
        if (user && user != this && !isFollowing(user))
        { // Prevent self-following and duplicate follows
            following.push_back(user);
            user->addFollower(this); // Add this user to the followed user's followers
        }
    }

    // Add a follower
    void addFollower(User *user)
    {
        if (!isFollowedBy(user))
        { // Prevent duplicate followers
            followers.push_back(user);
        }
    }

    // Check if already following a user
    bool isFollowing(User *user) const
    {
        for (User *u : following)
        {
            if (u == user)
                return true;
        }
        return false;
    }

    // Check if followed by a user
    bool isFollowedBy(User *user) const
    {
        for (User *u : followers)
        {
            if (u == user)
                return true;
        }
        return false;
    }

    void addPost(Post *post)
    {
        posts.push_back(post);
    }

    vector<User *> &getFollowing() { return following; }
    vector<User *> &getFollowers() { return followers; }

    vector<Post *> getContents() const { return posts; }

    void displayProfile() const
    {
        cout << "\033[1;34mUser: \033[0m" << username << endl;
        cout << "\033[1;32mFollowing: \033[0m" << this->following.size() << endl;
        cout << "\033[1;32mFollowers: \033[0m" << this->followers.size() << endl;
        cout << "\033[1;32mTotal posts: \033[0m" << this->posts.size() << endl;
    }

    void displayFollowers() const
    {
        cout << "\033[1;33mFollowers: \033[0m";
        for (auto u : followers)
        {
            cout << u->getUsername() << endl;
        }
        cout << endl;
    }

    void displayFollowing() const
    {
        cout << "\033[1;33mFollowing: \033[0m";
        for (auto u : following)
        {
            cout << u->getUsername() << endl;
        }
        cout << endl;
    }

    void displayContents() const
    {
        if (posts.empty())
        {
            cout << "\033[1;31mNo posts to display!\033[0m" << endl;
        }
        else
        {
            cout << "\033[1;36mAll posts: \033[0m" << endl;
            for (auto post : posts)
            {
                post->display();
            }
            cout << endl;
        }
    }
    // Save the user to a CSV file, including the following and followers list
    void saveToFile(ofstream &file) const
    {
        file << username << ",";
        for (User *u : following)
        {
            file << u->getUsername() << "|"; // Save following list
        }
        file << ","; // Separate followers
        for (User *u : followers)
        {
            file << u->getUsername() << "|"; // Save followers list
        }
        file << endl;
    }

    static User *findUser(const vector<User *> &users, const string &username)
    {
        for (User *u : users)
        {
            if (u->getUsername() == username)
                return u;
        }
        return nullptr;
    }
    static vector<User *> loadFromFile();
};

void Post ::display() const
{
    cout << "\033[1;34m" << author->getUsername() << "'s Post: \033[0m" << "\033[1;37m" << text << "\033[0m" << endl
         << "\033[1;32m" << likes << " likes\033[0m" << endl;
}

void Post ::saveToFile(ofstream &file) const
{
    file << author->getUsername() << "," << text << "," << likes << endl;
}
vector<User *> User::loadFromFile()
{
    vector<User *> users;
    ifstream file("users.csv");
    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string username, followingList, followersList;
        getline(ss, username, ',');
        getline(ss, followingList, ',');
        getline(ss, followersList, ',');

        User *newUser = new User(username);
        users.push_back(newUser);
    }

    // After loading users, parse the following and followers lists
    file.clear();
    file.seekg(0);
    int userIndex = 0;
    while (getline(file, line))
    {
        stringstream ss(line);
        string username, followingList, followersList;
        getline(ss, username, ',');
        getline(ss, followingList, ',');
        getline(ss, followersList, ',');

        User *currentUser = users[userIndex];

        // Process following list
        stringstream followStream(followingList);
        string followedUsername;
        while (getline(followStream, followedUsername, '|'))
        {
            User *followedUser = findUser(users, followedUsername);
            if (followedUser)
                currentUser->follow(followedUser);
        }

        // Process followers list
        stringstream followersStream(followersList);
        string followerUsername;
        while (getline(followersStream, followerUsername, '|'))
        {
            User *followerUser = findUser(users, followerUsername);
            if (followerUser)
                followerUser->follow(currentUser);
        }

        userIndex++;
    }
    file.close();
    return users;
}

vector<Post *> Post::loadFromFile(vector<User *> &users)
{
    vector<Post *> posts;
    ifstream file("posts.csv");
    string line, username, content;
    int likes;
    while (getline(file, line))
    {
        stringstream ss(line);
        getline(ss, username, ',');
        getline(ss, content, ',');
        ss >> likes;

        User *author = User::findUser(users, username);
        if (author)
        {
            Post *post = new Post(content, likes, author);
            posts.push_back(post);
            author->addPost(post);
        }
    }
    file.close();
    return posts;
}

class SocialMedia
{
private:
    vector<User *> users;
    vector<Post *> posts;

public:
    SocialMedia()
    {
        users = User::loadFromFile();
        posts = Post::loadFromFile(users);
    }

    void addUser(string username)
    {
        if (findUser(username))
        {
            cout << "User with username " << username << " already exists." << endl;
        }
        else
        {
            User *newUser = new User(username);
            users.push_back(newUser);
            saveUsersToFile();
        }
    }
    void sortUsersByFollowers()
    {
        sort(users.begin(), users.end(), [](User *a, User *b)
             { return a->getFollowers().size() > b->getFollowers().size(); });
    }
    User *findUser(string username) const
    {
        for (User *u : users)
        {
            if (u->getUsername() == username)
                return u;
        }
        return nullptr;
    }

    void createPost(User *user, string content)
    {
        Post *newPost = new Post(content, user);
        posts.push_back(newPost);
        user->addPost(newPost);
        savePostsToFile();
    }
    void displaySinglePost(Post *post) const
    {
        if (post)
        {
            system("clear");
            post->display();
            cout << "\033[1;36m1. Like\n2. Next\n3. Previous\n4. Home\033[0m" << endl;
        }
    }

    void displayPublicFeed() const
    {
        int currentIndex = 0;
        vector<Post *> allPosts;

        // Collect all posts
        for (User *user : users)
        {
            vector<Post *> userPosts = user->getContents();
            allPosts.insert(allPosts.end(), userPosts.begin(), userPosts.end());
        }

        if (allPosts.empty())
        {
            cout << "\033[1;31mNo posts to display!\033[0m" << endl;
            return;
        }

        while (true)
        {
            displaySinglePost(allPosts[currentIndex]);

            int choice;
            cout << "\033[1;33mEnter choice: \033[0m";
            cin >> choice;

            switch (choice)
            {
            case 1:
                allPosts[currentIndex]->like();
                savePostsToFile();
                break;
            case 2:
                if (currentIndex < allPosts.size() - 1)
                {
                    currentIndex++;
                }
                else
                {
                    cout << "\033[1;31mYou've reached the last post!\033[0m" << endl;
                }
                break;
            case 3:
                if (currentIndex > 0)
                {
                    currentIndex--;
                }
                else
                {
                    cout << "\033[1;31mYou're at the first post!\033[0m" << endl;
                }
                break;
            case 4:
                return;
            }
        }
    }

    void displayFollowedFeed(User *user) const
    {
        vector<Post *> followedPosts;
        int currentIndex = 0;

        // Collect posts from followed users
        for (User *followed : user->getFollowing())
        {
            vector<Post *> userPosts = followed->getContents();
            followedPosts.insert(followedPosts.end(), userPosts.begin(), userPosts.end());
        }

        if (followedPosts.empty())
        {
            cout << "\033[1;31mNo posts from followed users!\033[0m" << endl;
            cout << "\033[1;35mPress Enter to continue...\033[0m";
            cin.ignore();
            cin.get();
            return;
        }

        while (true)
        {
            displaySinglePost(followedPosts[currentIndex]);

            int choice;
            cout << "\033[1;33mEnter choice: \033[0m";
            cin >> choice;

            switch (choice)
            {
            case 1:
                followedPosts[currentIndex]->like();
                savePostsToFile();
                break;
            case 2:
                if (currentIndex < followedPosts.size() - 1)
                {
                    currentIndex++;
                }
                else
                {
                    cout << "\033[1;31mYou've reached the last post!\033[0m" << endl;
                }
                break;
            case 3:
                if (currentIndex > 0)
                {
                    currentIndex--;
                }
                else
                {
                    cout << "\033[1;31mYou're at the first post!\033[0m" << endl;
                }
                break;
            case 4:
                return;
            default:
                cout << "\033[1;31mInvalid choice. Please try again.\033[0m" << endl;
                break;
            }
        }
    }

    void saveUsersToFile() const
    {
        ofstream file("users.csv", ios::out);
        for (User *u : users)
        {
            u->saveToFile(file);
        }
        file.close();
    }

    void savePostsToFile() const
    {
        ofstream file("posts.csv", ios::out);
        for (Post *p : posts)
        {
            p->saveToFile(file);
        }
        file.close();
    }
};

int main()
{
    SocialMedia app;

    int choice;
    cout << "\033[1;36m 1. Login\n 2. Signup\033[0m" << endl;
    cout << "\033[1;33m Enter choice: \033[0m";
    cin >> choice;

    string username;
    cout << "\033[1;33m Enter username: \033[0m";
    cin >> username;

    User *currentUser = nullptr;

    if (choice == 1)
    {
        currentUser = app.findUser(username);
        if (currentUser)
        {
            cout << "\033[1;32m------ Login successful!------\033[0m" << endl
                 << endl;
            cout << "\033[1;34mWelcome \033[0m" << "\033[1;36m" << currentUser->getUsername() << "\033[0m" << endl
                 << endl;
            cout << "\033[1;35mPress Enter to continue...\033[0m";
            cin.ignore();
            cin.get();
        }
        else
        {
            cout << "User not found. Please sign up." << endl;
            return 0;
        }
    }
    else if (choice == 2)
    {
        app.addUser(username);
        currentUser = app.findUser(username);
        cout << "Signup successful!" << endl;
        cout << "\033[1;35mPress Enter to continue...\033[0m";
        cin.ignore();
        cin.get();
    }
    else
    {
        cout << "Invalid choice." << endl;
        return 0;
    }
    while (true)
    {
        system("clear");
        cout << "\033[1;36m1. Feed\033[0m" << endl;
        cout << "\033[1;36m2. Search User\033[0m" << endl;
        cout << "\033[1;36m3. Display Profile\033[0m" << endl;
        cout << "\033[1;36m4. Create Post\033[0m" << endl;
        cout << "\033[1;36m5. Exit\033[0m" << endl;
        cout << "\033[1;33mEnter your choice: \033[0m";
        cin >> choice;
        if (choice == 1)
        {
            cout << "\033[1;36m1. For You\033[0m" << endl;
            cout << "\033[1;36m2. Following\033[0m" << endl;
            cout << "\033[1;33mSelect feed type: \033[0m";
            int feedChoice;
            cin >> feedChoice;

            if (feedChoice == 1)
            {
                cout << endl;
                app.displayPublicFeed();
                cout << endl;
            }
            else if (feedChoice == 2)
            {
                cout << endl;
                app.displayFollowedFeed(currentUser);
                cout << endl;
            }
            else
            {
                cout << "\033[1;31mInvalid choice.\033[0m" << endl;
            }
        }

        else if (choice == 2)
        {
            string username;
            cout << "\033[1;33mEnter username to search: \033[0m";
            cin >> username;
            User *searchedUser = app.findUser(username);
            if (searchedUser)
            {
                bool staying = true;
                while (staying)
                {
                    system("clear");
                    searchedUser->displayProfile();
                    cout << "\033[1;36m1. Follow\033[0m" << endl;
                    cout << "\033[1;36m2. See Followers List\033[0m" << endl;
                    cout << "\033[1;36m3. See Following List\033[0m" << endl;
                    cout << "\033[1;36m4. See All Posts\033[0m" << endl;
                    cout << "\033[1;36m5. Back to Main Menu\033[0m" << endl;
                    cout << "\033[1;33mEnter your choice: \033[0m";
                    int userChoice;
                    cin >> userChoice;

                    switch (userChoice)
                    {
                    case 1:
                        if (currentUser->isFollowing(searchedUser))
                        {
                            cout << "\033[1;36mYou are already following : " << searchedUser->getUsername() << "\033[0m" << endl;
                            cout << "\033[1;33mEnter 1 to Unfollow or any other key to cancel: \033[0m";
                            int choice;
                            cin >> choice;
                            if (choice == 1)
                            {
                                auto &following = const_cast<vector<User *> &>(currentUser->getFollowing());
                                auto &followers = const_cast<vector<User *> &>(searchedUser->getFollowers());
                                following.erase(remove(following.begin(), following.end(), searchedUser), following.end());
                                followers.erase(remove(followers.begin(), followers.end(), currentUser), followers.end());
                                app.saveUsersToFile();
                                cout << "\033[1;32mYou are no longer following " << searchedUser->getUsername() << "!\033[0m" << endl;
                            }
                        }
                        else
                        {
                            currentUser->follow(searchedUser);
                            app.saveUsersToFile();
                            cout << "\033[1;32mYou are now following " << searchedUser->getUsername() << "!\033[0m" << endl;
                        }
                        app.saveUsersToFile();
                        cout << "\033[1;35mPress Enter to continue...\033[0m";
                        cin.ignore();
                        cin.get();
                        break;
                    case 2:
                        searchedUser->displayFollowers();
                        cout << "\033[1;35mPress Enter to continue...\033[0m";
                        cin.ignore();
                        cin.get();
                        break;
                    case 3:
                        searchedUser->displayFollowing();
                        cout << "\033[1;35mPress Enter to continue...\033[0m";
                        cin.ignore();
                        cin.get();
                        break;
                    case 4:
                        searchedUser->displayContents();
                        cout << "\033[1;35mPress Enter to continue...\033[0m";
                        cin.ignore();
                        cin.get();
                        break;
                    case 5:
                        staying = false;
                        break;
                    default:
                        cout << "\033[1;31mInvalid choice. Please try again.\033[0m" << endl;
                        cout << "\033[1;35mPress Enter to continue...\033[0m";
                        cin.ignore();
                        cin.get();
                    }
                }
            }
            else
            {
                cout << "\033[1;31mUser not found.\033[0m" << endl;
                cout << "\033[1;35mPress Enter to continue...\033[0m";
                cin.ignore();
                cin.get();
            }
        }
        else if (choice == 3)
        {
            bool staying = true;
            while (staying)
            {
                system("clear");
                cout << endl
                     << "\033[1;34m--------- User Profile ----------\033[0m" << endl;
                currentUser->displayProfile();
                cout << "\033[1;36m1. See Followers List\033[0m" << endl;
                cout << "\033[1;36m2. See Following List\033[0m" << endl;
                cout << "\033[1;36m3. See All Posts\033[0m" << endl;
                cout << "\033[1;36m4. Back to Main Menu\033[0m" << endl;
                cout << "\033[1;33mEnter your choice: \033[0m";
                int userChoice;
                cin >> userChoice;

                switch (userChoice)
                {
                case 1:
                    currentUser->displayFollowers();
                    cout << "\033[1;35mPress Enter to continue...\033[0m";
                    cin.ignore();
                    cin.get();
                    break;
                case 2:
                    currentUser->displayFollowing();
                    cout << "\033[1;35mPress Enter to continue...\033[0m";
                    cin.ignore();
                    cin.get();
                    break;
                case 3:
                    currentUser->displayContents();
                    cout << "\033[1;35mPress Enter to continue...\033[0m";
                    cin.ignore();
                    cin.get();
                    break;
                case 4:
                    staying = false;
                    break;
                default:
                    cout << "\033[1;31mInvalid choice. Please try again.\033[0m" << endl;
                    cout << "\033[1;35mPress Enter to continue...\033[0m";
                    cin.ignore();
                    cin.get();
                }
            }
        }
        else if (choice == 4)
        {
            string content;
            cout << "\033[1;33mEnter your post content: \033[0m";
            cin.ignore();
            getline(cin, content);
            app.createPost(currentUser, content);
            cout << "\033[1;32mPost created successfully!\033[0m" << endl;
            cout << "\033[1;35mPress Enter to continue...\033[0m";
            cin.get();
        }
        else if (choice == 5)
        {
            cout << endl
                 << endl
                 << "\033[1;31m------ Exiting... -------\033[0m" << endl;
            break;
        }
        else
        {
            cout << "\033[1;31mInvalid choice. Please try again.\033[0m" << endl;
            cout << "\033[1;35mPress Enter to continue...\033[0m";
            cin.ignore();
            cin.get();
        }
    }

    return 0;
}