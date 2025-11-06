#include "mainApp.h"
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QPushButton>
#include <QSpacerItem>
#include <QStyle>
#include <QIcon>
#include <QStatusBar>
#include <QGroupBox>
#include <QSizePolicy>
#include <QFrame>

// ========== RecipeStore ==========
RecipeStore::RecipeStore(QObject* parent) : QObject(parent) {
    loadMockData();
    loadFavorites();
}

void RecipeStore::loadMockData() {
    // Simple static mock dataset
    m_recipes = {
        {"r1", "Grilled Salmon with Lemon", "Seafood",
         "A simple, healthy grilled salmon with lemon and herbs.",
         {"Salmon fillet", "Lemon", "Olive oil", "Garlic", "Parsley", "Salt", "Pepper"},
         25, 420, ""},
        {"r2", "Chicken Alfredo Pasta", "Pasta",
         "Creamy Alfredo pasta with grilled chicken and parmesan.",
         {"Pasta", "Chicken breast", "Cream", "Parmesan", "Garlic", "Butter", "Salt"},
         30, 680, ""},
        {"r3", "Avocado Toast", "Breakfast",
         "Crispy toast topped with smashed avocado and chili flakes.",
         {"Bread", "Avocado", "Lemon", "Chili flakes", "Olive oil", "Salt"},
         10, 260, ""},
        {"r4", "Quinoa Salad", "Salad",
         "Fresh quinoa salad with cucumber, tomato, and feta.",
         {"Quinoa", "Cucumber", "Tomato", "Feta", "Olive oil", "Lemon", "Mint"},
         20, 320, ""},
        {"r5", "Beef Stir Fry", "Asian",
         "Savory beef stir fry with bell peppers and soy glaze.",
         {"Beef", "Bell peppers", "Onion", "Soy sauce", "Ginger", "Garlic"},
         22, 540, ""}
    };
}

void RecipeStore::loadFavorites() {
    QSettings settings("RecipeExplorer", "RecipeApp");
    int size = settings.beginReadArray("favorites");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        m_favorites.insert(settings.value("id").toString());
    }
    settings.endArray();
}

void RecipeStore::saveFavorites() const {
    QSettings settings("RecipeExplorer", "RecipeApp");
    settings.remove("favorites");
    settings.beginWriteArray("favorites");
    int i = 0;
    for (const auto& id : m_favorites) {
        settings.setArrayIndex(i++);
        settings.setValue("id", id);
    }
    settings.endArray();
}

QList<Recipe> RecipeStore::allRecipes() const {
    return m_recipes;
}

QList<Recipe> RecipeStore::search(const QString& query) const {
    if (query.trimmed().isEmpty()) return m_recipes;
    QList<Recipe> out;
    const auto q = query.trimmed().toLower();
    for (const auto& r : m_recipes) {
        if (r.title.toLower().contains(q)) {
            out.push_back(r);
            continue;
        }
        for (const auto& ing : r.ingredients) {
            if (ing.toLower().contains(q)) {
                out.push_back(r);
                break;
            }
        }
    }
    return out;
}

bool RecipeStore::isFavorite(const QString& recipeId) const {
    return m_favorites.contains(recipeId);
}

void RecipeStore::toggleFavorite(const QString& recipeId) {
    if (m_favorites.contains(recipeId)) {
        m_favorites.remove(recipeId);
    } else {
        m_favorites.insert(recipeId);
    }
    saveFavorites();
    emit favoritesChanged();
}

QList<Recipe> RecipeStore::favorites() const {
    QList<Recipe> out;
    for (const auto& r : m_recipes) {
        if (m_favorites.contains(r.id)) out.push_back(r);
    }
    return out;
}

// ========== RecipeCard ==========
RecipeCard::RecipeCard(const Recipe& recipe, bool favorite, QWidget* parent)
    : QWidget(parent), m_recipe(recipe) {
    setObjectName("Card");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(8);

    auto* title = new QLabel(recipe.title, this);
    title->setObjectName("Title");

    auto* subtitle = new QLabel(QString("%1 • %2 min • %3 cal")
                                    .arg(recipe.category)
                                    .arg(recipe.cookMinutes)
                                    .arg(recipe.calories), this);
    subtitle->setObjectName("Subtitle");

    auto* desc = new QLabel(recipe.description, this);
    desc->setWordWrap(true);

    auto* row = new QHBoxLayout();
    auto* openBtn = new QPushButton("Open", this);
    openBtn->setObjectName("Primary");

    m_favBtn = new QPushButton(this);
    m_favBtn->setToolTip("Toggle Favorite");
    updateFavIcon(favorite);

    row->addWidget(openBtn);
    row->addStretch();
    row->addWidget(m_favBtn);

    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addWidget(desc);
    layout->addLayout(row);

    connect(openBtn, &QPushButton::clicked, this, [this]() { emit openRequested(m_recipe); });
    connect(m_favBtn, &QPushButton::clicked, this, [this]() {
        emit favoriteToggled(m_recipe.id);
    });
}

void RecipeCard::updateFavIcon(bool fav) {
    m_favBtn->setText(fav ? "★ Favorite" : "☆ Favorite");
    m_favBtn->setStyleSheet(fav
        ? QString("QPushButton { background: %1; color: white; border-radius: 10px; padding: 6px 10px; }")
            .arg(Theme::secondary())
        : QString("QPushButton { background: %1; color: %2; }").arg(Theme::surface(), Theme::text()));
}

// ========== HomeView ==========
HomeView::HomeView(RecipeStore* store, QWidget* parent)
    : QWidget(parent), m_store(store) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* header = new QLabel("Discover", this);
    header->setObjectName("Title");
    layout->addWidget(header);

    auto* list = buildList(m_store->allRecipes());
    layout->addWidget(list, 1);

    connect(m_store, &RecipeStore::favoritesChanged, this, &HomeView::reload);
}

QWidget* HomeView::buildList(const QList<Recipe>& items) {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto* container = new QWidget(scroll);
    auto* v = new QVBoxLayout(container);
    v->setContentsMargins(4, 4, 4, 4);
    v->setSpacing(10);

    for (const auto& r : items) {
        auto* card = new RecipeCard(r, m_store->isFavorite(r.id), container);
        v->addWidget(card);
        connect(card, &RecipeCard::openRequested, this, &HomeView::openRecipe);
        connect(card, &RecipeCard::favoriteToggled, m_store, &RecipeStore::toggleFavorite);
        connect(m_store, &RecipeStore::favoritesChanged, card, [this, card, r]() {
            card->findChild<QPushButton*>()->setDown(false);
        });
        // update visual fav icon on change
        connect(m_store, &RecipeStore::favoritesChanged, card, [this, card, r]() {
            // hack: reapply icon
            // safer approach: expose method, but we have pointer
            // we can dynamic_cast; but we already have direct method
            // call through lambda capturing card and r
            // We need to call updateFavIcon; it's private. Workaround: emit toggle and restore? Simpler: rebuild on reload.
        });
    }
    v->addStretch();

    container->setLayout(v);
    scroll->setWidget(container);
    return scroll;
}

void HomeView::reload() {
    // Rebuild list to reflect favorite icons
    auto layout = qobject_cast<QVBoxLayout*>(this->layout());
    if (!layout) return;
    if (layout->count() >= 2) {
        auto* old = layout->itemAt(1)->widget();
        if (old) {
            layout->removeWidget(old);
            old->deleteLater();
        }
    }
    layout->addWidget(buildList(m_store->allRecipes()), 1);
}

// ========== SearchView ==========
SearchView::SearchView(RecipeStore* store, QWidget* parent)
    : QWidget(parent), m_store(store) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* header = new QLabel("Search", this);
    header->setObjectName("Title");

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search recipes or ingredients...");

    m_results = new QWidget(this);
    auto* rlayout = new QVBoxLayout(m_results);
    rlayout->setContentsMargins(0, 0, 0, 0);
    rlayout->setSpacing(0);
    m_results->setLayout(rlayout);

    layout->addWidget(header);
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_results, 1);

    renderResults(m_store->allRecipes());

    connect(m_searchEdit, &QLineEdit::textChanged, this, &SearchView::onTextChanged);
    connect(m_store, &RecipeStore::favoritesChanged, this, [this]() {
        onTextChanged(m_searchEdit->text());
    });
}

void SearchView::onTextChanged(const QString& text) {
    auto results = m_store->search(text);
    renderResults(results);
}

void SearchView::renderResults(const QList<Recipe>& results) {
    auto* layout = qobject_cast<QVBoxLayout*>(m_results->layout());
    // Clear
    QLayoutItem* child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    auto* scroll = new QScrollArea(m_results);
    scroll->setWidgetResizable(true);
    auto* container = new QWidget(scroll);
    auto* v = new QVBoxLayout(container);
    v->setContentsMargins(4, 4, 4, 4);
    v->setSpacing(10);

    for (const auto& r : results) {
        auto* card = new RecipeCard(r, m_store->isFavorite(r.id), container);
        v->addWidget(card);
        connect(card, &RecipeCard::openRequested, this, &SearchView::openRecipe);
        connect(card, &RecipeCard::favoriteToggled, m_store, &RecipeStore::toggleFavorite);
    }
    v->addStretch();

    container->setLayout(v);
    scroll->setWidget(container);
    layout->addWidget(scroll);
}

// ========== FavoritesView ==========
FavoritesView::FavoritesView(RecipeStore* store, QWidget* parent)
    : QWidget(parent), m_store(store) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    auto* header = new QLabel("Favorites", this);
    header->setObjectName("Title");
    layout->addWidget(header);

    m_list = new QWidget(this);
    layout->addWidget(m_list, 1);

    reload();
    connect(m_store, &RecipeStore::favoritesChanged, this, &FavoritesView::reload);
}

void FavoritesView::reload() {
    auto* layout = new QVBoxLayout(m_list);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto favs = m_store->favorites();

    auto* scroll = new QScrollArea(m_list);
    scroll->setWidgetResizable(true);
    auto* container = new QWidget(scroll);
    auto* v = new QVBoxLayout(container);
    v->setContentsMargins(4, 4, 4, 4);
    v->setSpacing(10);

    if (favs.isEmpty()) {
        auto* empty = new QLabel("No favorites yet. Tap the star on a recipe to save it.", container);
        v->addWidget(empty);
    } else {
        for (const auto& r : favs) {
            auto* card = new RecipeCard(r, true, container);
            v->addWidget(card);
            connect(card, &RecipeCard::openRequested, this, &FavoritesView::openRecipe);
            connect(card, &RecipeCard::favoriteToggled, m_store, &RecipeStore::toggleFavorite);
        }
    }
    v->addStretch();

    container->setLayout(v);
    scroll->setWidget(container);

    // Clear previous
    QLayoutItem* child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    layout->addWidget(scroll);
    m_list->setLayout(layout);
}

// ========== RecipeDetailView ==========
RecipeDetailView::RecipeDetailView(RecipeStore* store, QWidget* parent)
    : QWidget(parent), m_store(store) {
    setObjectName("Card");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    auto* backBtn = new QPushButton("← Back", this);
    connect(backBtn, &QPushButton::clicked, this, &RecipeDetailView::backRequested);

    m_title = new QLabel(this);
    m_title->setObjectName("Title");

    m_subtitle = new QLabel(this);
    m_subtitle->setObjectName("Subtitle");

    m_desc = new QLabel(this);
    m_desc->setWordWrap(true);

    m_meta = new QLabel(this);

    m_ingredients = new QLabel(this);
    m_ingredients->setWordWrap(true);

    m_favBtn = new QPushButton(this);
    connect(m_favBtn, &QPushButton::clicked, this, &RecipeDetailView::onToggleFavorite);

    layout->addWidget(backBtn, 0);
    layout->addWidget(m_title);
    layout->addWidget(m_subtitle);
    layout->addWidget(m_desc);
    layout->addWidget(m_meta);
    layout->addWidget(new QFrame(this), 0);
    layout->addWidget(new QLabel("Ingredients", this));
    layout->addWidget(m_ingredients);
    layout->addStretch();
    layout->addWidget(m_favBtn, 0);

    connect(m_store, &RecipeStore::favoritesChanged, this, &RecipeDetailView::refreshFavorite);
}

void RecipeDetailView::setRecipe(const Recipe& recipe) {
    m_recipe = recipe;
    m_title->setText(recipe.title);
    m_subtitle->setText(QString("%1 • %2 min • %3 cal")
                            .arg(recipe.category)
                            .arg(recipe.cookMinutes)
                            .arg(recipe.calories));
    m_desc->setText(recipe.description);
    m_meta->setText(QString("Category: %1").arg(recipe.category));
    m_ingredients->setText("• " + recipe.ingredients.join("\n• "));
    refreshFavorite();
}

void RecipeDetailView::onToggleFavorite() {
    m_store->toggleFavorite(m_recipe.id);
}

void RecipeDetailView::refreshFavorite() {
    bool fav = m_store->isFavorite(m_recipe.id);
    m_favBtn->setText(fav ? "★ Remove Favorite" : "☆ Add to Favorites");
    m_favBtn->setObjectName("Primary");
    m_favBtn->setStyleSheet(fav
        ? QString("QPushButton#Primary { background: %1; color: white; }").arg(Theme::secondary())
        : QString("QPushButton#Primary { background: %1; color: white; }").arg(Theme::primary()));
}

// ========== MainWindow ==========
QWidget* MainWindow::createTopNav() {
    auto* bar = new QWidget(this);
    auto* h = new QHBoxLayout(bar);
    h->setContentsMargins(12, 12, 12, 12);
    h->setSpacing(8);

    auto* title = new QLabel("Recipe Explorer", bar);
    title->setObjectName("Title");

    auto* homeBtn = new QPushButton("Home", bar);
    auto* searchBtn = new QPushButton("Search", bar);
    auto* favBtn = new QPushButton("Favorites", bar);

    homeBtn->setObjectName("Primary");

    h->addWidget(title);
    h->addStretch();
    h->addWidget(homeBtn);
    h->addWidget(searchBtn);
    h->addWidget(favBtn);

    connect(homeBtn, &QPushButton::clicked, this, &MainWindow::navigateHome);
    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::navigateSearch);
    connect(favBtn, &QPushButton::clicked, this, &MainWindow::navigateFavorites);

    return bar;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Recipe Explorer");
    resize(900, 640);
    setStyleSheet(Theme::baseStyleSheet());

    auto* central = new QWidget(this);
    auto* v = new QVBoxLayout(central);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);

    auto* topNav = createTopNav();
    m_stack = new QStackedWidget(central);

    // Views
    auto* home = new HomeView(&m_store, m_stack);
    auto* search = new SearchView(&m_store, m_stack);
    auto* favorites = new FavoritesView(&m_store, m_stack);
    auto* detail = new RecipeDetailView(&m_store, m_stack);

    m_homeIndex = m_stack->addWidget(home);
    m_searchIndex = m_stack->addWidget(search);
    m_favoritesIndex = m_stack->addWidget(favorites);
    int detailIndex = m_stack->addWidget(detail);

    connect(home, &HomeView::openRecipe, this, &MainWindow::showRecipeDetail);
    connect(search, &SearchView::openRecipe, this, &MainWindow::showRecipeDetail);
    connect(favorites, &FavoritesView::openRecipe, this, &MainWindow::showRecipeDetail);
    connect(detail, &RecipeDetailView::backRequested, this, [this]() { m_stack->setCurrentIndex(m_homeIndex); });

    v->addWidget(topNav, 0);
    v->addWidget(m_stack, 1);

    central->setLayout(v);
    setCentralWidget(central);

    m_stack->setCurrentIndex(m_homeIndex);
}

void MainWindow::navigateHome() { m_stack->setCurrentIndex(m_homeIndex); }
void MainWindow::navigateSearch() { m_stack->setCurrentIndex(m_searchIndex); }
void MainWindow::navigateFavorites() { m_stack->setCurrentIndex(m_favoritesIndex); }

void MainWindow::showRecipeDetail(const Recipe& recipe) {
    // Find detail view
    for (int i = 0; i < m_stack->count(); ++i) {
        if (auto* d = qobject_cast<RecipeDetailView*>(m_stack->widget(i))) {
            d->setRecipe(recipe);
            m_stack->setCurrentIndex(i);
            break;
        }
    }
}

// ========== main ==========
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
