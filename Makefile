# Цели сборки
DB_TARGET = program
URL_CLIENT_TARGET = url_shortener
URL_SERVER_TARGET = url_server
URL_GUI_TARGET = url_shortener_gui

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRCDIR = src

# Исходные файлы сервера базы данных
DB_SRC = $(SRCDIR)/main.cpp \
         $(SRCDIR)/database/database.cpp \
         $(SRCDIR)/database/threadsafe_db.cpp \
         $(SRCDIR)/network/network_server.cpp \
         $(SRCDIR)/network/client_connection.cpp \
         $(SRCDIR)/network/thread_pool.cpp \
         $(SRCDIR)/utils/logger.cpp \
         $(SRCDIR)/structures/set.cpp \
         $(SRCDIR)/structures/stack.cpp \
         $(SRCDIR)/structures/queue.cpp \
         $(SRCDIR)/structures/hash.cpp \
         $(SRCDIR)/structures/tree.cpp

# Исходные файлы клиента URL shortener
URL_CLIENT_SRC = $(SRCDIR)/url_shortener_main.cpp

# Исходные файлы сервера URL shortener
URL_SERVER_SRC = $(SRCDIR)/url_server_main.cpp \
                 $(SRCDIR)/http/http_server.cpp \
                 $(SRCDIR)/http/request_handler.cpp \
                 $(SRCDIR)/shortener/url_shortener_service.cpp \
                 $(SRCDIR)/shortener/database_client.cpp \
                 $(SRCDIR)/network/thread_pool.cpp \
                 $(SRCDIR)/utils/logger.cpp

# Исходные файлы GUI приложения
URL_GUI_SRC = $(SRCDIR)/gui/url_shortener_gui.mm

DB_OBJ = $(DB_SRC:.cpp=.o)
URL_CLIENT_OBJ = $(URL_CLIENT_SRC:.cpp=.o)
URL_SERVER_OBJ = $(URL_SERVER_SRC:.cpp=.o)
URL_GUI_OBJ = $(URL_GUI_SRC:.mm=.o)

# По умолчанию собираем клиент URL shortener
all: $(URL_CLIENT_TARGET)

# Сборка сервера базы данных
$(DB_TARGET): $(DB_OBJ)
	$(CXX) $(CXXFLAGS) $(DB_OBJ) -o $(DB_TARGET)

# Сборка клиента URL shortener
$(URL_CLIENT_TARGET): $(URL_CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) $(URL_CLIENT_OBJ) -o $(URL_CLIENT_TARGET)

# Сборка сервера URL shortener
$(URL_SERVER_TARGET): $(URL_SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $(URL_SERVER_OBJ) -o $(URL_SERVER_TARGET)

# Сборка GUI приложения
$(URL_GUI_TARGET): $(URL_GUI_OBJ)
	clang++ -std=c++17 -framework Cocoa $(URL_GUI_OBJ) -o $(URL_GUI_TARGET)

# Компиляция объектных файлов
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.mm
	clang++ -std=c++17 -framework Cocoa -c $< -o $@

# Очистка
clean:
	rm -f $(DB_OBJ) $(URL_CLIENT_OBJ) $(URL_SERVER_OBJ) $(URL_GUI_OBJ) $(DB_TARGET) $(URL_CLIENT_TARGET) $(URL_SERVER_TARGET) $(URL_GUI_TARGET)

clean-db:
	rm -f $(DB_OBJ) $(DB_TARGET)

clean-url:
	rm -f $(URL_CLIENT_OBJ) $(URL_CLIENT_TARGET)

clean-server:
	rm -f $(URL_SERVER_OBJ) $(URL_SERVER_TARGET)

clean-gui:
	rm -f $(URL_GUI_OBJ) $(URL_GUI_TARGET)

clean-obj:
	rm -f $(DB_OBJ) $(URL_CLIENT_OBJ) $(URL_SERVER_OBJ) $(URL_GUI_OBJ)

# Пересборка
rebuild: clean all
rebuild-db: clean-db $(DB_TARGET)
rebuild-url: clean-url $(URL_CLIENT_TARGET)
rebuild-server: clean-server $(URL_SERVER_TARGET)

# Сборка с отладочной информацией
dev: CXXFLAGS += -g -DDEBUG_MODE
dev: $(URL_CLIENT_TARGET)

dev-db: CXXFLAGS += -g -DDEBUG_MODE
dev-db: $(DB_TARGET)

dev-server: CXXFLAGS += -g -DDEBUG_MODE
dev-server: $(URL_SERVER_TARGET)

# Оптимизированная сборка
release: CXXFLAGS += -O3 -DNDEBUG
release: $(URL_CLIENT_TARGET)

release-db: CXXFLAGS += -O3 -DNDEBUG
release-db: $(DB_TARGET)

release-server: CXXFLAGS += -O3 -DNDEBUG
release-server: $(URL_SERVER_TARGET)

# Сборка всех компонентов
both: $(DB_TARGET) $(URL_SERVER_TARGET) $(URL_CLIENT_TARGET)
all-gui: $(DB_TARGET) $(URL_SERVER_TARGET) $(URL_GUI_TARGET)

# Запуск компонентов
run-db: $(DB_TARGET)
	./$(DB_TARGET)

run-url: $(URL_CLIENT_TARGET)
	@echo "Использование: ./$(URL_CLIENT_TARGET) <URL>"
	@echo "Пример: ./$(URL_CLIENT_TARGET) https://google.com"

run-server: $(URL_SERVER_TARGET)
	./$(URL_SERVER_TARGET)

run-gui: $(URL_GUI_TARGET)
	./$(URL_GUI_TARGET)

# Установка
install: $(URL_CLIENT_TARGET)
	@echo "Установка URL shortener клиента в /usr/local/bin (требуется sudo)"
	sudo cp $(URL_CLIENT_TARGET) /usr/local/bin/

# Справка
help:
	@echo "Доступные цели:"
	@echo "  all              - Собрать клиент URL shortener (по умолчанию)"
	@echo "  $(DB_TARGET)            - Собрать сервер базы данных"
	@echo "  $(URL_CLIENT_TARGET)     - Собрать клиент URL shortener"
	@echo "  $(URL_GUI_TARGET)        - Собрать GUI приложение"
	@echo "  all-gui          - Собрать сервер БД, URL сервер и GUI"
	@echo "  both             - Собрать все компоненты"
	@echo "  dev              - Собрать клиент с отладочной информацией"
	@echo "  dev-db           - Собрать сервер БД с отладочной информацией"
	@echo "  dev-server       - Собрать URL сервер с отладочной информацией"
	@echo "  release          - Собрать оптимизированный клиент"
	@echo "  release-db       - Собрать оптимизированный сервер БД"
	@echo "  release-server   - Собрать оптимизированный URL сервер"
	@echo "  clean            - Удалить все файлы сборки"
	@echo "  clean-db         - Удалить файлы сборки сервера БД"
	@echo "  clean-url        - Удалить файлы сборки клиента"
	@echo "  clean-server     - Удалить файлы сборки URL сервера"
	@echo "  clean-obj        - Удалить только объектные файлы"
	@echo "  rebuild          - Очистить и пересобрать клиент"
	@echo "  rebuild-db       - Очистить и пересобрать сервер БД"
	@echo "  rebuild-url      - Очистить и пересобрать клиент"
	@echo "  rebuild-server   - Очистить и пересобрать URL сервер"
	@echo "  run-db           - Собрать и запустить сервер БД"
	@echo "  run-url          - Показать использование клиента"
	@echo "  run-gui          - Собрать и запустить GUI"
	@echo "  install          - Установить клиент в систему (требуется sudo)"
	@echo "  help             - Показать эту справку"

.PHONY: all clean clean-db clean-url clean-server clean-obj rebuild rebuild-db rebuild-url rebuild-server \
        dev dev-db dev-server release release-db release-server both \
        run-db run-url run-server install help