#include "utils.h"
#include <stdio.h>

int is_text_file(const char *filename)
{
	FILE *f = fopen(filename, "r");
	if (!f)
		return 0; // Dosya açılamıyorsa başarısız bildirisi döner.

	int c;
	// Dosyanın sonuna gelene kadar tek tek tüm baytları okuyoruz.
	while ((c = fgetc(f)) != EOF)
	{
		// Eğer okuduğumuz baytlardan herhangi biri 0 ise, bu dosya metin dosyası olamaz.
		// Çünkü metin formatlarında NULL (0) baytı bulunmaz. Öyle bir durumda binary (exe, jpg vb.) olduğunu anlayıp 0 döndürüyoruz.
		if (c == 0)
		{
			fclose(f);
			return 0;
		}
	}

	// Dosyada hiç 0 baytına rastlanmadıysa geçerli dosya bilgisi döner.
	fclose(f);
	return 1;
}