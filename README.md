# IsletimSistemleriProje_Grup32


## İçindekiler

- [Giriş](#giriş)
- [Özellikler](#özellikler)
- [Kurulum](#kurulum)
  - [Önkoşullar](#önkoşullar)
  - [Adımlar](#adımlar)
- [Kullanım](#kullanım)
  - [Kabuk Programını Çalıştırma](#kabuk-programını-çalıştırma)
  - [Temel Komutlar](#temel-komutlar)
  - [Boru (`|`)](#boru-)
  - [Komut Sıralaması (`;`)](#komut-sıralaması-)
  - [Arka Plan Çalıştırma (`&`)](#arka-plan-çalıştırma--)
  - [Girdi/Çıktı Yönlendirme](#girdiçıktı-yönlendirme)
- [Örnekler](#örnekler)
- [Geliştirme](#geliştirme)
  - [Makefile](#makefile)
- [Katkıda Bulunma](#katkıda-bulunma)
- [Lisans](#lisans)
- [İletişim](#iletişim)

## Giriş

 C dilinde yazılmış basit bir UNIX kabuk programıdır. Bu proje, kabuk geliştirme konusundaki temel kavramları, komut ayrıştırma, süreç yönetimi, Girdi/Çıktı yönlendirme, borulama (piping) ve arka plan süreçlerinin yönetimi gibi konuları göstermek amacıyla eğitim amaçlıdır. `bash` veya `zsh` gibi gelişmiş kabukların tüm özelliklerini kapsamazken, kabukların nasıl çalıştığını anlamak için sağlam bir temel sunar.

## Özellikler

- **Komut Çalıştırma:** Standart UNIX komutlarını çalıştırma yeteneği.
- **Girdi/Çıktı Yönlendirme:** Girdi (`<`), çıktı (`>`), ve ekleme (`>>`) yönlendirmelerini destekler.
- **Boru (`|`):** Bir komutun çıktısını başka bir komutun girdisine yönlendirme.
- **Komut Sıralaması (`;`):** Birden fazla komutu sıralı olarak çalıştırma.
- **Arka Plan Çalıştırma (`&`):** Komutları arka planda çalıştırma.
- **Sinyal Yönetimi:** Kesintileri düzgün bir şekilde ele alır ve arka planda çalışan süreçleri yönetir.
- **Özel Yerleşik Komutlar:**
  - `cd`: Mevcut dizini değiştirme.
  - `quit`: Kabuktan çıkış yapma.
- **Yardımcı Program:**
  - `increment`: Standart girdiden aldığı sayıyı bir artırarak çıktıya yazan basit bir program.

## Kurulum

### Önkoşullar

- **GCC Derleyicisi:** `gcc` yüklü olmalıdır. Aşağıdaki komutlarla yükleyebilirsiniz:

  ```bash
  sudo apt-get update
  sudo apt-get install build-essential
