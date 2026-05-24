#include "tarsau.h"
#include "extract.h"

// Gelen klasör yolu "klasor1/klasor2" gibi katmanlıysa, önce klasor1'i sonra klasor2'yi oluşturan yardımcı fonksiyon.
static void create_nested_dir(const char *path)
{
	char temp[512];
	snprintf(temp, sizeof(temp), "%s", path);

	// Yoldaki '/' karakterlerini bulup oraya kadar olan kısmı klasör olarak oluşturuyoruz.
	for (char *p = temp + 1; *p; p++)
	{
		if (*p == '/')
		{
			*p = '\0';
			mkdir(temp, 0777); // Klasörü tam yetkiyle (0777) oluşturur.
			*p = '/';
		}
	}
	mkdir(temp, 0777); // Yolun en sonundaki asıl klasörü oluşturur.
}

int extract_archive(const char *archive_name, const char *dir_name)
{
	// Verilen dosyanın isminin son 4 harfine bakıp uzantısı gerçekten ".sau" mu diye kontrol ediyoruz.
	int len = strlen(archive_name);
	if (len < 5 || strcmp(archive_name + len - 4, ".sau") != 0)
	{
		printf("Arşiv dosyası uygunsuz veya bozuk!\n");
		return 0;
	}

	FILE *in = fopen(archive_name, "r");
	if (!in)
	{
		printf("Arşiv dosyası uygunsuz veya bozuk!\n");
		return 0;
	}

	// Arşivin en başındaki ilk 10 karakteri okuyoruz. Sayı 10 adet belirlenmişti.
	char size_str[11];
	if (fread(size_str, 1, 10, in) != 10)
	{
		printf("Arşiv dosyası uygunsuz veya bozuk!\n");
		fclose(in);
		return 0;
	}
	size_str[10] = '\0';

	// İlk 10 karakterin içinde harf vb. varsa arşiv bozulmuş demektir, sadece rakam olmak zorunda.
	for (int i = 0; i < 10; i++)
	{
		if (size_str[i] < '0' || size_str[i] > '9')
		{
			printf("Arşiv dosyası uygunsuz veya bozuk!\n");
			fclose(in);
			return 0;
		}
	}

	// Bilgi kısmının (metadata) boyutunu sayıya çeviriyoruz ki ne kadar bayt daha okuyacağımızı bilelim.
	size_t metadata_size = (size_t)atoi(size_str) - 10;
	if (metadata_size <= 0)
	{
		printf("Arşiv dosyası uygunsuz veya bozuk!\n");
		fclose(in);
		return 0;
	}

	// Bilgileri RAM'de tutmak için hesapladığımız boyut kadar yer ayırıp orayı arşivden tamamen okuyoruz.
	char *metadata = malloc(metadata_size + 1);
	if (fread(metadata, 1, metadata_size, in) != metadata_size)
	{
		printf("Arşiv dosyası uygunsuz veya bozuk!\n");
		free(metadata);
		fclose(in);
		return 0;
	}
	metadata[metadata_size] = '\0';

	// Kullanıcı dosyaları bir klasöre çıkarmak istediyse o klasörleri oluşturuyoruz.
	if (dir_name != NULL)
	{
		create_nested_dir(dir_name);

		struct stat st;
		// Klasör başarıyla oluşturuldu mu ve gerçekten bir klasör mü diye sağlamasını yapıyoruz.
		if (stat(dir_name, &st) != 0 || !S_ISDIR(st.st_mode))
		{
			printf("Arşiv dosyası uygunsuz veya bozuk!\n");
			free(metadata);
			fclose(in);
			return 0;
		}
	}

	// strtok fonksiyonu ile metadata stringini '|' işaretinden bölerek her bir dosyanın bilgisine ulaşıyoruz.
	char *token = strtok(metadata, "|");
	while (token != NULL)
	{
		char fname[256];
		unsigned int mode_octal;
		long fsize;

		// Her parçanın içinden sscanf ile dosyanın ismini, octal(8'lik) izin değerini ve boyutunu çekip alıyoruz.
		if (sscanf(token, "%[^,],%o,%ld", fname, &mode_octal, &fsize) != 3)
		{
			printf("Arşiv dosyası uygunsuz veya bozuk!\n");
			free(metadata);
			fclose(in);
			return 0;
		}

		// Dosyanın çıkarılacağı tam yolu hazırlıyoruz (Klasör ismi varsa başına ekliyoruz ve araya '/' koyuyoruz).
		char full_path[512];
		if (dir_name != NULL)
		{
			int dlen = strlen(dir_name);
			if (dir_name[dlen - 1] == '/')
			{
				sprintf(full_path, "%s%s", dir_name, fname);
			}
			else
			{
				sprintf(full_path, "%s/%s", dir_name, fname);
			}
		}
		else
		{
			strcpy(full_path, fname);
		}

		// Çıkartılacak dosyayı dizinde oluşturuyoruz.
		FILE *out = fopen(full_path, "w");
		if (!out)
		{
			printf("Arşiv dosyası uygunsuz veya bozuk!\n");
			free(metadata);
			fclose(in);
			return 0;
		}

		// Arşivden sadece bu dosyanın boyutu kadar bayt okuyup dizinde oluşturduğumuz yeni dosyaya kopyalıyoruz.
		for (long i = 0; i < fsize; i++)
		{
			int c = fgetc(in);
			// Boyutu tamamlamadan dosya sonu gelirse arşiv eksik/bozuk demektir.
			if (c == EOF)
			{
				printf("Arşiv dosyası uygunsuz veya bozuk!\n");
				fclose(out);
				free(metadata);
				fclose(in);
				return 0;
			}
			fputc(c, out);
		}
		fclose(out);

		// Kopyalama bittikten sonra dosyanın arşivlenmeden önceki eski okuma/yazma izinlerini ona geri veriyoruz.
		chmod(full_path, mode_octal);

		// Diğer dosyanın bilgisine geçmek için token'ı güncelliyoruz.
		token = strtok(NULL, "|");
	}

	free(metadata);
	fclose(in);
	return 1;
}