#include "tarsau.h"
#include "utils.h"
#include "archive.h"

int create_archive(char *input_files[], int input_count, const char *output_name)
{
	// Sınırımız 32 dosya. Kullanıcı daha fazlasını verdiyse işlemi en başından iptal ediyoruz.
	if (input_count > MAX_FILES)
	{
		fprintf(stderr, "Hata: Giriş dosyası sayısı en fazla 32 olabilir.\n");
		return 0;
	}

	long total_size = 0;
	struct stat st;

	// Arşivi oluşturmaya başlamadan önce, verilen tüm dosyaları tek tek kontrol ediyoruz ki ileride hata çıkmasın.
	for (int i = 0; i < input_count; i++)
	{
		// stat() ile dosya gerçekten diskte var mı ona bakıyoruz. Yoksa hata verir.
		if (stat(input_files[i], &st) != 0)
		{
			printf("Hata: '%s' dosyası sistemde bulunamadı!\n", input_files[i]);
			return 0;
		}

		// 2. S_ISREG ile bunun bir klasör değil, normal bir dosya olduğunu teyit edilir.
		if (!S_ISREG(st.st_mode))
		{
			printf("Hata: '%s' normal bir dosya değil!\n", input_files[i]);
			return 0;
		}

		// 3. İçinde 0 baytı var mı (yani binary bir dosya mı, değil mi) kontrolü utils.c'deki fonksiyona yaptırılır ve duruma göre hata gönderilir.
		if (!is_text_file(input_files[i]))
		{
			printf("Hata: '%s' giriş dosyasının formatı uyumsuzdur!\n", input_files[i]);
			return 0;
		}

		// Dosya geçerliyse boyutunu toplam değere ekliyoruz.
		total_size += st.st_size;
	}

	// Seçilen tüm dosyaların boyutları toplamı 200 MB'ı geçiyorsa işlem reddediliyor.
	if (total_size > MAX_TOTAL_SIZE)
	{
		fprintf(stderr, "Hata: Giriş dosyalarının toplam boyutu 200 MB'ı geçemez.\n");
		return 0;
	}

	// Dosyaların isimlerini, boyutlarını ve izinlerini tutacağımız bilgi satırını başlatıyoruz. Ayraç olarak en başa '|' koyduk.
	char metadata[16384] = "|";
	for (int i = 0; i < input_count; i++)
	{
		stat(input_files[i], &st);
		char entry[512];

		// st.st_mode'un içinden sadece dosya izin kısımlarını çekip alıyoruz.
		int perms = st.st_mode & 0777;

		// Araya virgül koyarak "dosya_adı,izin,boyut|" formatında stringi hazırlıyoruz ve ana listeye (metadata) ekliyoruz.
		sprintf(entry, "%s,%o,%ld|", input_files[i], perms, (long)st.st_size);
		strcat(metadata, entry);
	}

	// Bu noktada artık her şey hazır, çıktı (.sau uzantılı) dosyasını oluşturmak üzere yazma(w) modunda açıyoruz.
	FILE *out = fopen(output_name, "w");
	if (!out)
	{
		fprintf(stderr, "Hata: Çıktı dosyası oluşturulamadı.\n");
		return 0;
	}

	// Dosyanın en başına tam 10 karakter kaplayacak şekilde metadata'nın toplam uzunluğunu yazıyoruz.
	// Okurken ne kadarlık kısmı okuyacağımızı buradan bileceğiz.
	int total_header_size = 10 + strlen(metadata);
	fprintf(out, "%010d", total_header_size);
	fputs(metadata, out);

	// Burada da dosyaların içindeki asıl içeriği arşive ekleme işlemi yapılıyor.
	for (int i = 0; i < input_count; i++)
	{
		FILE *in = fopen(input_files[i], "r");
		if (!in)
			continue;

		// Dosyanın içindeki karakterleri teker teker okuyup arşive yazıyoruz.
		int c;
		while ((c = fgetc(in)) != EOF)
		{
			fputc(c, out);
		}
		fclose(in);
	}

	fclose(out);
	return 1;
}