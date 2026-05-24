#include "tarsau.h"
#include "archive.h"
#include "extract.h"

int main(int argc, char *argv[])
{
	// Kullanıcı program isminin yanına en az 2 şey daha yazmalı (-b ve dosya ismi gibi), yazmamışsa kullanımı göster.
	if (argc < 3)
	{
		fprintf(stderr, "Komutunuz hatalıdır. Geçerli Kullanım:\n  tarsau -b <dosyalar> -o <arsiv_adi.sau>\n  tarsau -a <arsiv_adi.sau> [hedef_dizin]\n");
		return 1;
	}

	// Kullanıcı "-b" parametresi girdiyse arşivleme işlemi başlatılıyor.
	if (strcmp(argv[1], "-b") == 0)
	{
		char *input_files[MAX_FILES];
		int input_count = 0;
		char *output_name = "a.sau"; // Kullanıcı -o parametresini unutursa dosya a.sau adıyla çıksın.

		// Komut satırındaki argümanları tarıyoruz (İlk iki elemanı; program adını ve "-b"yi atlayıp direkt dosyalardan başlıyoruz).
		for (int i = 2; i < argc; i++)
		{
			if (strcmp(argv[i], "-o") == 0)
			{
				// -o gördüysek, yanındaki parametre bizim çıktı dosyamızın adı olmalı. Onu alıyoruz.
				if (i + 1 < argc)
				{
					output_name = argv[i + 1];
					i++; // Çıktı ismini normal dosya sanıp diziye eklemesin diye 'i'yi bir artırıp üzerinden atlıyoruz.
				}
				else
				{
					fprintf(stderr, "Hata: -o parametresinden sonra dosya adı belirtilmedi.\n");
					return 1;
				}
			}
			else
			{
				// -o haricinde ne varsa arşivlenecek dosyalardır. Sınırı (32) aşmıyorsa listemize ekliyoruz.
				if (input_count < MAX_FILES)
				{
					input_files[input_count++] = argv[i];
				}
				else
				{
					fprintf(stderr, "Hata: En fazla 32 dosya eklenebilir.\n");
					return 1;
				}
			}
		}

		// Bütün parametreleri gezmiş isek ama ortada hiç girdi dosyası kalmadıysa hata fırlatıyoruz.
		if (input_count == 0)
		{
			fprintf(stderr, "Hata: Arşivlenecek giriş dosyası belirtilmedi.\n");
			return 1;
		}

		// Listelediğimiz dosyaları ve belirlediğimiz arşiv adını archive.c içindeki fonksiyona yolluyoruz.
		create_archive(input_files, input_count, output_name);

		// Kullanıcı "-a" parametresi girdiyse extract işlemi başlatılıyor.
	}
	else if (strcmp(argv[1], "-a") == 0)
	{
		// -a işleminde en fazla "tarsau -a arsiv.sau klasor" şeklinde 4 argüman olabilir. Fazlası yanlış kullanımdır.
		if (argc > 4)
		{
			fprintf(stderr, "Hata: Çok fazla parametre belirtildi.\n");
			return 1;
		}

		// -a'nın hemen yanındaki argüman okunacak olan arşivin ismidir.
		char *archive_name = argv[2];

		// Eğer 4 argüman varsa 4. argüman hedef klasördür, 4 argüman yoksa klasör ismi girmemiştir NULL yollarız.
		char *dir_name = (argc == 4) ? argv[3] : NULL;

		// İsimleri extract.c içindeki çıkarma fonksiyonuna yolluyoruz.
		extract_archive(archive_name, dir_name);
	}
	else
	{
		// Kullanıcı ne -a ne de -b girmiş. Hata mesajı gönderiyoruz.
		fprintf(stderr, "Hata: Geçersiz parametre. -b veya -a kullanın.\n");
		return 1;
	}

	return 0;
}