CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude # C kodunu derlerken tüm uyarıları göster ve include klasörünü kütüphane olarak kullan.

BIN_DIR = bin
SRC_DIR = src
LIB_DIR = lib

TARGET = $(BIN_DIR)/tarsau

# src klasörünün içindeki '.c' uzantılı tüm dosya isimlerini toplayıp SRCS listesine atar.
SRCS = $(wildcard $(SRC_DIR)/*.c)
# SRCS listesindeki tüm isimleri alır, başındaki src/ yi lib/ yapar, sonundaki .c yi .o yapar ve OBJS listesine yazar.
OBJS = $(patsubst $(SRC_DIR)/%.c, $(LIB_DIR)/%.o, $(SRCS))

# Terminalde sadece 'make' yazınca ilk bu çalışır. Gider önce 'clean' komutunu çalıştırır her yeri temizler, 
# sonra aşağıda $(TARGET) hedefine gidip sıfırdan derlemeye başlar.
all: clean
	@echo "Önceki artıklar temizlendi, derleme süreci başlatılıyor..."
	@$(MAKE) $(TARGET) --no-print-directory

# Elimizdeki '.o' uzantılı (makine diline çevrilmiş) tüm dosyaları birbirine yapıştırarak son çalışan dosyayı oluşturur.
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	@echo "Derleme tamamlandı: $(TARGET)"

# src klasöründeki her bir '.c' dosyasını alıp teker teker makine diline çevirir ve lib klasörüne '.o' olarak kaydeder.
$(LIB_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Programı derlerken oluşan tüm geçici .o dosyalarını, çıkan asıl dosyayı ve test arşivini silip çöpe atar.
clean:
	@rm -f $(LIB_DIR)/*.o
	@rm -rf $(BIN_DIR)/*
	@rm -f a.sau
	@echo "Temizlik tamamlandı."

# 'all' ve 'clean' isimleri, projemizde gerçekten dosyalar olsa bile kodumuz komut gibi algılansın diye burada belirtilir.
.PHONY: all clean