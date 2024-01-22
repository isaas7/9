#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <gtk/gtk.h>
#include <iostream>
namespace asio = boost::asio;
namespace ssl = boost::asio::ssl;
namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;

void client(const std::string &host, const std::string &port,
            const std::string &target, const std::string &payload,
            const std::function<void(std::string)> &callback) {
  asio::io_context io;
  ssl::context ctx(ssl::context::sslv23_client);
  tcp::resolver resolver(io);
  beast::tcp_stream stream(io);

  auto const results = resolver.resolve(host, port);
  stream.connect(results);
  beast::http::request<beast::http::string_body> req(beast::http::verb::post,
                                                     target, 11);
  req.set(beast::http::field::host, host);
  req.set(beast::http::field::user_agent, "Boost Beast Client");
  req.set(beast::http::field::content_type, "application/json");
  req.body() = payload;
  req.prepare_payload();

  beast::http::write(stream, req);

  beast::flat_buffer buffer;
  beast::http::response<beast::http::string_body> res;
  beast::http::read(stream, buffer, res);
  std::string sessionToken = res[beast::http::field::authorization];
  std::cout << "sessionToken: " << sessionToken << std::endl;
  callback(sessionToken);
}

void showDashboard(gpointer data) {
  GtkWidget *mainWindow =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "mainWindow"));
  GtkWidget *usernameEntry =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "usernameEntry"));
  GtkWidget *passwordEntry =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "passwordEntry"));
  GtkWidget *loginButton =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "loginButton"));
  gtk_widget_hide(usernameEntry);
  gtk_widget_hide(passwordEntry);
  gtk_widget_hide(loginButton);
  GtkWidget *dashboardLabel = gtk_label_new("Welcome to the Dashboard!");
  GtkWidget *fixed =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "fixed1"));
  gtk_fixed_put(GTK_FIXED(fixed), dashboardLabel, 70, 30);
  gtk_widget_show_all(mainWindow);
}
void showError(gpointer data) {
  GtkWidget *mainWindow =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "mainWindow"));
  GtkWidget *errorLabel = gtk_label_new("Login failed. Please try again.");
  GtkWidget *fixed =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "fixed1"));
  gtk_fixed_put(GTK_FIXED(fixed), errorLabel, 70, 120);

  gtk_widget_show_all(mainWindow);
}

void onButtonClicked(GtkWidget *button, gpointer data) {
  GtkWidget *usernameEntry =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "usernameEntry"));
  GtkWidget *passwordEntry =
      GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(data), "passwordEntry"));

  const gchar *username = gtk_entry_get_text(GTK_ENTRY(usernameEntry));
  const gchar *password = gtk_entry_get_text(GTK_ENTRY(passwordEntry));
  std::string jsonPayload = "{ \"username\": \"" + std::string(username) +
                            "\", \"password\": \"" + std::string(password) +
                            "\" }";
  client("localhost", "8080", "/api/user/login", jsonPayload,
         [data](const std::string &sessionToken) {
           if (!sessionToken.empty()) {
             showDashboard(data);
           } else {
             showError(data);
           }
         });
}

int main(int argc, char **argv) {
  gtk_init(&argc, &argv);
  GtkBuilder *builder = gtk_builder_new_from_file("ui_file.glade");
  GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
  GtkWidget *button =
      GTK_WIDGET(gtk_builder_get_object(builder, "loginButton"));
  g_signal_connect(button, "clicked", G_CALLBACK(onButtonClicked), builder);
  gtk_widget_show_all(window);
  gtk_main();
  return 0;
}
