# Проект множество Мандельброта

---

## Содержание

1. [Введение](#введение)
2. [Отрисовка множества](#отрисовка-множества)
3. [Палитра](#палитра-мандельброта)
4. [Наивный подход к вычислению итераций](#наивное-вычисление-количества-итераций-до-выхода)
5. [Оптимизация SIMD](#оптимизация-simd)
6. [Оптимизация массивами](#оптимизация-массивами)
7. [Про тестирование](#про-тестирование)
8. [Вывод](#вывод)

--- 

## Введение

Множество Мандельброта - это множество точек **с** принадлежащих комплексному полю, для которых рекурентное соотношение $`z_{n+1}=z_{n}^2 + c`$ 
при $`z_0 = 0`$ задаёт ограниченную последовательность. Подробнее про данное множество можно почитать [на википедии](https://en.wikipedia.org/wiki/Mandelbrot_set).
Этот проект направлен на отрисовку множества и оптимизацию вычислений точек множества мандельброта

---

## Отрисовка множества
  Чтобы отрисовывать изображение будем использовать библиотеку [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) написанную на C. Понятно, что у нас нет возможности исследовать каждую точку для бесконечного числа итераций, но мы вполне можем ограничиться некоторым числом итераций, после которого будем считать, что **c** принадлежит множеству (константа `MAX_ITERATIONS`) Для начала опишем структуру мандельброта, в которой будет содержаться его реальные размеры, координаты центра, зум, экран, а также палитру цветов
```cpp
typedef struct MandelbrotData
{
    int* iterations_per_pixel;
    uint32_t* colors[MAX_ITERATIONS];
    double zoom;
    double center_x;
    double center_y;
    double width;
    double height;    
} MandelbrotData;
```

### Палитра мандельброта

   Точки, принадлежащие мандельброту будем красить в черный цвет, для остальных точек заведём некоторую палитру с цветами для каждого количества итераций до выхода из радиуса множества. Например моя функция для генерации такой палитры:
```cpp
// задаем формат в котором будет храниться палитра
const SDL_PixelFormatDetails* format = NULL;
format = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);

for (int i = 0; i < MAX_ITERATIONS; i++)
{
    float t = i / (float)(MAX_ITERATIONS - 1);
    uint8_t r = 255 * sin(5 * (1 - t) * M_PI);
    uint8_t g = 255 * cos(3 * (1 - t) * M_PI);
    uint8_t b = 255 * sin(7 * (1 - t) * M_PI);
    // функция SDL_MapRGBA переводит переменные r, g, b в правильный формат
    data->colors[i] = SDL_MapRGBA(format, NULL, r, g, b, 255);
}
```

### Наивное вычисление количества итераций до выхода

Затем разберемся, как определить принадлежит ли пиксель на экране множеству. Попробуем для начала наивный подход, потом будем пытаться оптимизировать.
```cpp
// нормализуем данные относительно реальных размеров
double norm_x = (x_pixel / (double)SCREEN_WIDTH) * data->width;
double norm_y = ((SCREEN_HEIGHT - y_pixel) / (double)SCREEN_HEIGHT) * data->height;
// просчёт координат центра
const double x0 = norm_x - (data->width / 2) + data->center_x;
const double y0 = norm_y - (data->height / 2) + data->center_y;

double x2 = 0.0;
double y2 = 0.0;
double w = 0.0;

int iteration = 0;
while (x2 + y2 <= 4.0 && iteration < MAX_ITERATIONS)
{
    double x = x2 - y2 + x0;
    double y = w - x2 - y2 + y0;
    w = (x + y) * (x + y);
    x2 = x * x;
    y2 = y * y;
    iteration++;
}

return iteration;
```

### Цикл наивных вычислений

```cpp
int* field = data->iterations_per_pixel;

for (int y = 0; y < SCREEN_HEIGHT; y++)
{
    for (int x = 0; x < SCREEN_WIDTH; x++) 
    {
        int iterations = calculateIterationFromPosition(x, y, data);
        field[y * SCREEN_WIDTH + x] = iterations % MAX_ITERATIONS;
    }
}
```

---

## Оптимизация SIMD

Понятно, что код описанный выше будет работать не быстро. Поэтому появляется желание оптимизировать работу кода. Сущесвует немало способов как можно добиться лучшей скорости исполнения: несколько пототов, вычисление на GPU вместо CPU или использование [векторных инструкций](https://en.wikipedia.org/wiki/Single_instruction,_multiple_data).

Большинство современных процессоров поддерживают набор инструкций, называемый AVX2 ([Advanced Vector Extensions](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions)). AVX2 дает возможность считать количество итераций до выхода нескольких точек одновременно, например в моём случае 4 точки с точностью `double`. 

Для того, чтобы использовать эти инструкции в коде достаточно подключить библиотеку `#include <immintrin.h>` и передать компилятору флаг `-avx2`. Нам станут доступны функции назваемые *intrinsics*. Описание всех интринсиков можно прочитаьть на [оффициальном сайте intel](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/) или на [доступном зеркале](https://www.laruence.com/sse/).

Приступим к оптимизации кода. Функцию вычислений перепишем таким образом, чтобы одновременно считались 4 точки.
```cpp
__m256d x2 = _mm256_setzero_pd();
__m256d y2 = _mm256_setzero_pd();
__m256d w  = _mm256_setzero_pd();

__m256i iterations = _mm256_setzero_si256();
const __m256d max_radius = _mm256_set1_pd(4.0);

for (int i = 0; i < MAX_ITERATIONS; i++) {
    __m256d mask = _mm256_cmp_pd(_mm256_add_pd(x2, y2), max_radius, _CMP_LE_OQ);

    if (!_mm256_movemask_pd(mask)) 
    {
        break;
    }
    
    __m256d x = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
    __m256d y = _mm256_add_pd(_mm256_sub_pd(_mm256_sub_pd(w, x2), y2), y0);

    w = _mm256_mul_pd(_mm256_add_pd(x, y), _mm256_add_pd(x, y));

    x2 = _mm256_mul_pd(x, x);
    y2 = _mm256_mul_pd(y, y);

    iterations = _mm256_sub_epi64(iterations, _mm256_castpd_si256(mask));
}

return iterations;
```
Причем будем нормализовывать координаты 4 точек одновременно
```cpp
int* field = data->iterations_per_pixel;

const double dx = data->width / SCREEN_WIDTH;
const double dy = data->height / SCREEN_HEIGHT;
const __m256d offset_x = _mm256_set1_pd(data->center_x - data->width / 2);

for (int y = 0; y < SCREEN_HEIGHT; y++) 
{
    const double norm_y = (SCREEN_HEIGHT - y) * dy - data->height / 2 + data->center_y;
    const __m256d y0 = _mm256_set1_pd(norm_y);
    
    for (int x = 0; x < SCREEN_WIDTH; x += 8) 
    {
        __m256d x_pixels = _mm256_add_pd(
            _mm256_set1_pd(x),
            _mm256_set_pd(3.0, 2.0, 1.0, 0.0)
        );
        __m256d x0 = _mm256_fmadd_pd(x_pixels1, _mm256_set1_pd(dx), offset_x);
        __m128i iterations = calculateIterationsFromPositionIntrinsicsCastIter(x0, y0);

        _mm_store_si128(
            (__m128i*)(field + y * SCREEN_WIDTH + x),
            iterations
        );
    }
}
```

### Сравнение результатов SIMD и наивной версий

Замечательно, кажется, что стало работать быстрее. Узнаем насколько стало быстрее. Для этого будем считать тики процессора с помощью итринсика `__rdtsc` определённого в библиотеке `x86intrin.h`.

Тесты запускал локально, основная информация:

|                         |                                           |
|-------------------------|-------------------------------------------|
| Cистема                 | Arch Linux x86_64 (kernel 6.13.8-arch1-1) |
| CPU                     | AMD Ryzen 7 5700U                         |
| Опции компилятора       | `-O2`/`-O3` `-march=native`               |
| Компилятор              | gcc (GCC) 14.2.1                          |
| Средняя температура CPU | ~80 °C                                    |
| Средняя частота CPU     | ~4.3 Ghz                                  |

#### Результаты сравнения наивной версии и с использованием SIMD

Для флага `-O2`:
![compare_basic_simd_O2](/plots/comprasion_basic_version_O2_vs_simd_version_O2.png)

Для флага `-03`:
![compare_basic_simd_O3](/plots/comprasion_basic_version_O3_vs_simd_version_O3.png)
  
| Флаг      | Наивный подход       | SIMD                 |
|-----------|----------------------|----------------------|
| **-O2**   | $`5.919 \cdot 10^8`$ | $`1.543 \cdot 10^8`$ |
| **-O3**   | $`5.920 \cdot 10^8`$ | $`1.543 \cdot 10^8`$ |

В общем как примерно и ожидалось мы увеличили производительность в 4 раза. 

---

## Оптимизация массивами

Также один из способов улучшить производительность программы это работать с массивами вместо SIMD. Идея в том, чтобы дать компилятору возможность самому оптимизировать код. Для удобства работы будем использовать макросы. Тогда перепишем код просчета количества итераций.

```cpp
// ARRAY_SIZE = 16
double x2[ARRAY_SIZE] = {0.0};
double y2[ARRAY_SIZE] = {0.0};
double w[ARRAY_SIZE]  = {0.0};

int mask[ARRAY_SIZE] = {0};
bool active = false;

for (int i = 0; i < MAX_ITERATIONS; i++) 
{
    double radius[ARRAY_SIZE] = {0.0};
    ARRAY_AND_ARRAY_OP(+, radius, x2, y2, ARRAY_SIZE);
    ARRAY_COMPARE_AND_SET_MASK(<=, mask, radius, 4.0, active, ARRAY_SIZE);
    if (!active)
    {
        break;
    }
    
    double x[ARRAY_SIZE] = {0.0};
    double y[ARRAY_SIZE] = {0.0};

    ARRAY_AND_ARRAY_OP(-, x, x2, y2, ARRAY_SIZE)        
    ARRAY_AND_ARRAY_OP(+, x, x, x0, ARRAY_SIZE)

    ARRAY_AND_ARRAY_OP(-, y, w, x2, ARRAY_SIZE)
    ARRAY_AND_ARRAY_OP(-, y, y, y2, ARRAY_SIZE)
    ARRAY_AND_ARRAY_OP(+, y, y, y0, ARRAY_SIZE)

    ARRAY_AND_ARRAY_OP(*, x2, x, x, ARRAY_SIZE)
    ARRAY_AND_ARRAY_OP(*, y2, y, y, ARRAY_SIZE)
    ARRAY_AND_ARRAY_OP(+, w, x, y, ARRAY_SIZE)
    ARRAY_AND_ARRAY_OP(*, w, w, w, ARRAY_SIZE)

    ARRAY_AND_ARRAY_OP(+, iterations, iterations, mask, ARRAY_SIZE)
}
```
Часть кода, который сгенирировал компилятор
```asm
.L19:
	vmovd	%r15d, %xmm11
	vmovdqa	%ymm2, 192(%rsp)
	vmovdqa	%ymm2, 224(%rsp)
	vmovdqa	%ymm2, 256(%rsp)
	vmovdqa	%ymm2, 288(%rsp)
	vmovdqa	%ymm2, 320(%rsp)
	vmovdqa	%ymm2, 352(%rsp)
	vpbroadcastd	%xmm11, %ymm11
	vmovdqa	%ymm2, 384(%rsp)
	vmovdqa	%ymm2, 416(%rsp)
	vmovdqa	%ymm2, 64(%rsp)
	movq	%r13, %rbx
	vmovdqa	%ymm2, 96(%rsp)
	movq	%r14, %r11
	vmovdqa	%ymm8, %ymm10
.L3:
	vpaddd	%ymm6, %ymm10, %ymm12
	vpaddd	%ymm11, %ymm10, %ymm10
	addq	$64, %r11
	addq	$64, %rbx
	vextracti128	$0x1, %ymm10, %xmm13
	vcvtdq2pd	%xmm10, %ymm10
	vcvtdq2pd	%xmm13, %ymm13
	vfmadd132pd	%ymm5, %ymm4, %ymm10
	vfmadd132pd	%ymm5, %ymm4, %ymm13
	vmovapd	%ymm10, -64(%r11)
	vmovapd	%ymm13, -32(%r11)
	vmovapd	%ymm3, -64(%rbx)
	vmovapd	%ymm3, -32(%rbx)
	cmpq	%r11, %r13
```

Можно увидеть, что он активно использовать SIMD инструкции.

#### Производительность версии мандельброта на массивах

`-O2`:
![compare_basic_array_O2](/plots/comprasion_basic_version_O2_vs_array_version_O2.png)

`-O3`:
![compare_basic_array_O3](/plots/comprasion_basic_version_O3_vs_array_version_O3.png)

| Флаг      | Наивный подход       | Массивы              |
|-----------|----------------------|----------------------|
| **-O2**   | $`5.919 \cdot 10^8`$ | $`2.851 \cdot 10^8`$ |
| **-O3**   | $`5.920 \cdot 10^8`$ | $`3.291 \cdot 10^8`$ |

Можем увидеть, что производительность выросла в 2 раза, что тоже очень неплохой результат.

## Про тестирование

Тесты запускались на ноутбуке без графической оболочки, причем программа использовала для тестов только одно ядро. Функция вычисления итераций для SIMD и версии на массивах прогонялась по 10000 раз, плюс разогревочные запуски. Благодаря этому удалось получиться относительная погрешность для всех тестов составила меньше $`1\%`$.

## Вывод 

Даже без использования GPU и многопоточности, а только средствами компилятора, можно хорошо увеличить производительность наших программ. 

