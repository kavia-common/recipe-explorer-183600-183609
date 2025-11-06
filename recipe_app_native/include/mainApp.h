#ifndef MAIN_APP_H
#define MAIN_APP_H

#include <QWidget>
#include <QMainWindow>
#include <QStackedWidget>
#include <QList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>

// Added explicit includes for Qt types used as member pointers or values
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QSet>

// Small, focused data model for recipes used across views
struct Recipe {
    QString id;
    QString title;
    QString category;
    QString description;
    QStringList ingredients;
    int cookMinutes;
    int calories;
    QString image; // placeholder path or URL
};

// PUBLIC_INTERFACE
class Theme {
public:
    /** Holds global style variables for Ocean Professional theme. */
    static QString primary() { return "#2563EB"; }
    static QString secondary() { return "#F59E0B"; }
    static QString success() { return "#F59E0B"; }
    static QString error() { return "#EF4444"; }
    static QString background() { return "#f9fafb"; }
    static QString surface() { return "#ffffff"; }
    static QString text() { return "#111827"; }

    // PUBLIC_INTERFACE
    static QString baseStyleSheet() {
        /** Returns application-level stylesheet applying the Ocean Professional theme. */
        return QString(
            "QMainWindow { background: %1; }"
            "QWidget#Card { background: %2; border: 1px solid rgba(0,0,0,0.06); border-radius: 12px; }"
            "QLabel[objectName='Title'] { color: %3; font-size: 18px; font-weight: 600; }"
            "QLabel[objectName='Subtitle'] { color: #374151; font-size: 13px; }"
            "QPushButton { background: %1; color: %4; border: 1px solid rgba(0,0,0,0.08); border-radius: 10px; padding: 8px 12px; }"
            "QPushButton:hover { background: #eef2ff; }"
            "QPushButton#Primary { background: %5; color: white; border: none; }"
            "QPushButton#Primary:hover { background: #1e40af; }"
            "QLineEdit { background: %2; border: 1px solid rgba(0,0,0,0.12); border-radius: 10px; padding: 8px 10px; }"
            "QListWidget { background: %2; border: 1px solid rgba(0,0,0,0.06); border-radius: 12px; }"
            "QStatusBar { background: %2; border-top: 1px solid rgba(0,0,0,0.06); }"
        ).arg(background(), surface(), text(), text(), primary());
    }
};

// PUBLIC_INTERFACE
class RecipeStore : public QObject {
    Q_OBJECT
public:
    /** In-app data layer with mock recipes and favorites persistence via QSettings. */
    explicit RecipeStore(QObject* parent = nullptr);

    // PUBLIC_INTERFACE
    QList<Recipe> allRecipes() const;
    /** Returns all available mock recipes. */

    // PUBLIC_INTERFACE
    QList<Recipe> search(const QString& query) const;
    /** Returns recipes filtered by title or ingredient matching query. */

    // PUBLIC_INTERFACE
    bool isFavorite(const QString& recipeId) const;
    /** Returns whether a recipe is marked as favorite. */

    // PUBLIC_INTERFACE
    void toggleFavorite(const QString& recipeId);
    /** Toggles favorite state and persists to local device storage. */

    // PUBLIC_INTERFACE
    QList<Recipe> favorites() const;
    /** Returns list of favorite recipes. */

signals:
    void favoritesChanged();

private:
    QList<Recipe> m_recipes;
    QSet<QString> m_favorites;
    void loadMockData();
    void loadFavorites();
    void saveFavorites() const;
};

// Screens
class HomeView;
class SearchView;
class FavoritesView;
class RecipeDetailView;

// PUBLIC_INTERFACE
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    /** Main window hosting stack-based navigation and top navigation bar. */
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void navigateHome();
    void navigateSearch();
    void navigateFavorites();
    void showRecipeDetail(const Recipe& recipe);

private:
    RecipeStore m_store;
    QStackedWidget* m_stack;
    QWidget* createTopNav();
    int m_homeIndex{-1};
    int m_searchIndex{-1};
    int m_favoritesIndex{-1};
};

// Card widget used in lists
class RecipeCard : public QWidget {
    Q_OBJECT
public:
    explicit RecipeCard(const Recipe& recipe, bool favorite, QWidget* parent = nullptr);

signals:
    void openRequested(const Recipe& recipe);
    void favoriteToggled(const QString& recipeId);

private:
    Recipe m_recipe;
    QPushButton* m_favBtn{nullptr};
    void updateFavIcon(bool fav);
};

// Home
class HomeView : public QWidget {
    Q_OBJECT
public:
    explicit HomeView(RecipeStore* store, QWidget* parent = nullptr);

signals:
    void openRecipe(const Recipe& recipe);

private:
    RecipeStore* m_store;
    QWidget* buildList(const QList<Recipe>& items);
    void reload();
};

// Search
class SearchView : public QWidget {
    Q_OBJECT
public:
    explicit SearchView(RecipeStore* store, QWidget* parent = nullptr);

signals:
    void openRecipe(const Recipe& recipe);

private slots:
    void onTextChanged(const QString& text);

private:
    RecipeStore* m_store;
    QLineEdit* m_searchEdit{nullptr};
    QWidget* m_results{nullptr};
    void renderResults(const QList<Recipe>& results);
};

// Favorites
class FavoritesView : public QWidget {
    Q_OBJECT
public:
    explicit FavoritesView(RecipeStore* store, QWidget* parent = nullptr);

signals:
    void openRecipe(const Recipe& recipe);

private:
    RecipeStore* m_store;
    QWidget* m_list{nullptr};
    void reload();
};

// Detail
class RecipeDetailView : public QWidget {
    Q_OBJECT
public:
    explicit RecipeDetailView(RecipeStore* store, QWidget* parent = nullptr);
    void setRecipe(const Recipe& recipe);

signals:
    void backRequested();

private slots:
    void onToggleFavorite();

private:
    RecipeStore* m_store;
    Recipe m_recipe;
    QLabel* m_title{nullptr};
    QLabel* m_subtitle{nullptr};
    QLabel* m_desc{nullptr};
    QLabel* m_meta{nullptr};
    QLabel* m_ingredients{nullptr};
    QPushButton* m_favBtn{nullptr};
    void refreshFavorite();
};

#endif // MAIN_APP_H