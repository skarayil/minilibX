# 🎮 MLX Games - Interactive Graphics Learning Projects

<div align="center">

![42 School](https://img.shields.io/badge/School-42-black?style=for-the-badge&logo=42)
![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Graphics](https://img.shields.io/badge/MiniLibX-Graphics-blue?style=for-the-badge)
![Games](https://img.shields.io/badge/Games-3-success?style=for-the-badge)

**MiniLibX grafik kütüphanesi ile oyun geliştirme öğrenme projesi**

*Bu proje, grafik programlama, event handling, fizik simülasyonu ve oyun geliştirme prensiplerini öğretmek için tasarlanmış üç farklı zorluk seviyesinde oyun içerir.*

• [Oyunlar](#-oyunlar) • [Kurulum](#-kurulum) • [Kullanım](#-kullanım)

</div>

---

## 🎯 Projenin Amacı

<img align="right" alt="Gaming Animation" width="300" src="https://media.giphy.com/media/xUA7aM09ByyR1w5YWc/giphy.gif">

Bu proje, **MiniLibX grafik kütüphanesini öğrenmek** için üç farklı zorluk seviyesinde oyun içerir. Her oyun, farklı grafik programlama konseptlerini ve teknikleri öğretmek üzere tasarlanmıştır.

**⚠️ ÖNEMLİ NOT**

> Bu proje **MiniLibX'in OpenGL sürümü** ile çalışacak şekilde tasarlanmıştır.

---

## 🎮 Oyunlar

### 1️⃣ 🏓 **Breakout** - Başlangıç Seviyesi

<div align="center">

![Difficulty](https://img.shields.io/badge/Difficulty-Easy-brightgreen?style=for-the-badge)
![Concepts](https://img.shields.io/badge/Concepts-3-blue?style=for-the-badge)

</div>

#### 📝 **Açıklama**
Klasik Breakout oyunu - topla tuğlaları kırın, topu düşürmeyin!

#### 🎯 **Öğrenilen Konseptler**
- **Basic Rendering**: Dikdörtgen ve daire çizimi
- **Simple Collision**: AABB (Axis-Aligned Bounding Box) collision
- **Keyboard Input**: Arrow key handling
- **Game Loop**: Temel oyun döngüsü

#### 🎨 **Teknik Özellikler**
```
Window Size: 800x600
Brick Rows: 6
Brick Columns: 10
Paddle Speed: 8 pixels/frame
Ball Speed: 2.5x, 3.5y pixels/frame
Particle System: ✅ 200 particles
```

#### 🕹️ **Kontroller**
| Tuş | Fonksiyon |
|-----|-----------|
| **←** | Paddle sola |
| **→** | Paddle sağa |
| **SPACE** | Yeniden başlat (game over) |
| **ESC** | Çıkış |

#### 💎 **Özel Özellikler**
- Rainbow colored bricks (6 farklı renk)
- Particle explosion efekti
- Smooth paddle control
- Automatic brick regeneration

---

### 2️⃣ 🎯 **CatchMe** - Orta Seviye

<div align="center">

![Difficulty](https://img.shields.io/badge/Difficulty-Medium-orange?style=for-the-badge)
![Concepts](https://img.shields.io/badge/Concepts-5-blue?style=for-the-badge)

</div>

#### 📝 **Açıklama**
Hızla hareket eden kareyi fare ile yakalayın! Combo yaparak yüksek skor kazanın.

#### 🎯 **Öğrenilen Konseptler**
- **Mouse Events**: Click ve drag handling
- **Advanced Animation**: Smooth movement, trails, glow effects
- **Dynamic Difficulty**: Progressive speed increase
- **Combo System**: Time-based scoring
- **Visual Feedback**: Particles, glow, color cycling

#### 🎨 **Teknik Özellikler**
```
Window Size: 600x600
Game Duration: 30 seconds
Initial Speed: 3
Max Speed: 20
Particle System: ✅ 200 particles
Background Stars: ✅ 100 stars
Trail Effect: ✅ 20 positions
```

#### 🕹️ **Kontroller**
| Aksiyon | Fonksiyon |
|---------|-----------|
| **Sol Tık** | Kareyi yakala |
| **SPACE** | Yeniden başlat |

#### 💎 **Özel Özellikler**
- Rainbow colored target
- Glow ve trail efektleri
- Twinkling stars background
- Combo multiplier system
- Progressive difficulty
- Real-time statistics (zoom, center, render time)

---

### 3️⃣ 🚀 **Asteroids** - İleri Seviye

<div align="center">

![Difficulty](https://img.shields.io/badge/Difficulty-Hard-red?style=for-the-badge)
![Concepts](https://img.shields.io/badge/Concepts-8-blue?style=for-the-badge)

</div>

#### 📝 **Açıklama**
Uzayda hayatta kalın! Asteroidleri vurun, kuyruklu yıldızlardan kaçının, kalkan kullanın.

#### 🎯 **Öğrenilen Konseptler**
- **Complex Physics**: Velocity, acceleration, momentum
- **Advanced Collision**: Multiple object types, distance-based
- **Image Loading**: XPM file handling
- **Sprite Rendering**: Transparency ve rotation
- **Multiple Entities**: Asteroids, comets, lasers, particles
- **Background Effects**: Animated gradient, parallax stars
- **Shield Mechanic**: Temporary invulnerability
- **Time Management**: Survival timer, bonus time

#### 🎨 **Teknik Özellikler**
```
Window Size: 800x800
Asteroids: 8
Comets: 3
Max Lasers: 50
Laser Speed: 12 pixels/frame
Ship Speed: 2.5 max
Particle System: ✅ 200 particles
Star System: ✅ 80 parallax stars
Comet System: ✅ Trail effects
Shield System: ✅ Pulse animation
Game Duration: 60 seconds (extendable)
```

#### 🕹️ **Kontroller**
| Tuş | Fonksiyon |
|-----|-----------|
| **SPACE** | Ateş et |
| **Left SHIFT** | Kalkan aktif et |
| **SPACE** | Yeniden başlat (game over) |

#### 💎 **Özel Özellikler**
- Animated space background (gradient scroll)
- XPM rocket sprite with transparency
- Parallax star field with trails
- Comet system with trail effects
- Shield with pulse animation
- Laser trail effects
- Particle explosions
- Bonus time mechanics (+3s per asteroid, +5s per comet)
- Turkish language interface
- Death by comet detection

---

## 🚀 Kurulum

### 📥 **Kurulum Adımları**

```bash
# 1. Repository'yi klonlayın
git clone https://github.com/skarayil/minilibX.git
cd minilibX

# 2. Tüm oyunları derleyin
make

# 3. Oyunları çalıştırın
./breakout_game
./catchme_game
./asteroids_game
```

### 🔨 **Makefile Komutları**

```bash
make           # Tüm oyunları derle
make clean     # Object dosyalarını temizle
make fclean    # Tüm executable'ları sil
make re        # Yeniden derle
```

---

## 💻 Kullanım

### 🎮 **Oyunları Çalıştırma**

#### 🏓 **Breakout**
```bash
./breakout_game

# Hedef: Tüm tuğlaları kır
# İpucu: Topu paddle'ın kenarlarında yakala, açılı şut yap
```

#### 🎯 **CatchMe**
```bash
./catchme_game

# Hedef: 30 saniyede maksimum skor
# İpucu: Ard arda yakalama yaparak combo bonusu kazan
```

#### 🚀 **Asteroids**
```bash
./asteroids_game

# Hedef: 60 saniye hayatta kal
# İpucu: 
# - Asteroid vurarak +3 saniye kazan
# - Kuyruklu yıldızlara kalkan ile çarp: +5 saniye
# - Kalkanı stratejik kullan
```

---
<div align="center">
  
### 👨‍💻 Created by Sude Naz Karayıldırım

[![42 Profile](https://img.shields.io/badge/42%20Profile-skarayil-black?style=flat-square&logo=42&logoColor=white)](https://profile.intra.42.fr/users/skarayil)
[![GitHub](https://img.shields.io/badge/GitHub-skarayil-181717?style=flat-square&logo=github&logoColor=white)](https://github.com/skarayil)

**⭐ Eğer bu proje işinize yaradıysa, repo'ya star vermeyi unutmayın!**

**🎓 MLX öğrenmek isteyenler için mükemmel bir başlangıç noktası!**

</div>
