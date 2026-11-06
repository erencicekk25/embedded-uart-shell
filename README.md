# Gömülü UART Komut Satırı

Bu proje, UART üzerinden gelen komutları işleyen küçük bir C kabuğudur. Sabit boyutlu bellek kullanır ve dinamik bellek ayırmaz.

## Özellikler

- UART verisi için atomik indeksli halka tampon
- `help`, `set_param`, `get_status` ve `reboot` komutları
- Uzun satır ve tampon dolması için hata kontrolü
- Komut işleyicileri ve çıktı bağlantısı uygulamadan verilir
- Donanımdan bağımsız C11 çekirdeği

## Derleme

Gerekli araçlar C11 destekli bir C derleyicisi ve CMake 3.16 veya üstüdür.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Örnek kullanımı

Derlemeden sonra `shell_demo` uygulamasını çalıştırın. Bu uygulama UART yerine standart giriş ve çıkışı kullanır.

```text
> set_param threshold 250
OK: parameter updated.
> get_status
threshold=250, reboot=idle
> reboot
Reboot requested (simulation).
```

`reboot` örneği cihazı gerçekten yeniden başlatmaz. Gerçek üründe bu komut, platformun reset işlevine bağlanmalıdır.

## Donanıma bağlama

UART alım kesmesinde her gelen byte için `cli_receive_byte` çağrılır. Ana döngüde düzenli olarak `cli_process` çalıştırılır. `cli_init` fonksiyonuna verilen çıktı işlevi, metni UART sürücüsüne iletir.

Halka tampon tek üretici ve tek tüketici varsayar. Atomik byte işlemlerinin hedef işlemcide kilitsiz çalıştığı doğrulanmalıdır. Tamponda kullanılabilir alan 127 byte'tır; dolu olduğunda yeni byte kabul edilmez.

## Komut ekleme

Yeni komut için bir işleyici ve `cli_command_t` kaydı ekleyin. İşleyici, uygulama bağlamına ve çıktı API'sine erişebilir. Komutlar en fazla 8 argüman alır; satır uzunluğu sonlandırıcı byte dahil 64 byte ile sınırlıdır.