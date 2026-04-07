document.addEventListener('DOMContentLoaded', () => {
    // 1. Theme Toggle Logic
    const themeToggleBtn = document.getElementById('theme-toggle');
    const htmlElement = document.documentElement;
    
    const savedTheme = localStorage.getItem('theme');
    const systemPrefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    
    let currentTheme = savedTheme || (systemPrefersDark ? 'dark' : 'light');
    
    applyTheme(currentTheme);

    themeToggleBtn.addEventListener('click', () => {
        currentTheme = currentTheme === 'dark' ? 'light' : 'dark';
        applyTheme(currentTheme);
    });

    function applyTheme(theme) {
        if (theme === 'dark') {
            htmlElement.setAttribute('data-theme', 'dark');
            themeToggleBtn.innerHTML = '☀️ Light';
        } else {
            htmlElement.removeAttribute('data-theme');
            themeToggleBtn.innerHTML = '🌙 Dark';
        }
        localStorage.setItem('theme', theme);
    }

    // 2. Mobile Hamburger Menu
    const mobileMenuBtn = document.getElementById('mobile-menu');
    const navMenu = document.querySelector('.nav-menu');
    const navLinks = document.querySelectorAll('.nav-links');

    mobileMenuBtn.addEventListener('click', () => {
        navMenu.classList.toggle('active');
    });

    navLinks.forEach(link => {
        link.addEventListener('click', () => {
            if (navMenu.classList.contains('active')) {
                navMenu.classList.remove('active');
            }
        });
    });

    // 3. Smooth Scrolling
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            e.preventDefault();
            const targetId = this.getAttribute('href');
            const targetElement = document.querySelector(targetId);
            
            if (targetElement) {
                targetElement.scrollIntoView({
                    behavior: 'smooth'
                });
            }
        });
    });

    // 4. Gallery Carousel Logic
    const carouselInner = document.querySelector('.carousel-inner');
    const dots = document.querySelectorAll('.dot');
    const prevBtn = document.getElementById('prevBtn');
    const nextBtn = document.getElementById('nextBtn');
    
    if (carouselInner && dots.length > 0) {
        let currentSlide = 0;
        const totalSlides = dots.length;

        function updateCarousel(index) {
            // Check bounds for infinite loop logic
            if (index >= totalSlides) {
                currentSlide = 0;
            } else if (index < 0) {
                currentSlide = totalSlides - 1;
            } else {
                currentSlide = index;
            }

            // Move the inner container to show current slide
            carouselInner.style.transform = `translateX(-${currentSlide * 100}%)`;

            // Update active dot
            dots.forEach(dot => dot.classList.remove('active'));
            dots[currentSlide].classList.add('active');
        }

        // Button Listeners
        nextBtn.addEventListener('click', () => updateCarousel(currentSlide + 1));
        prevBtn.addEventListener('click', () => updateCarousel(currentSlide - 1));

        // Dot Listeners
        dots.forEach((dot, index) => {
            dot.addEventListener('click', () => updateCarousel(index));
        });
        
        // Optional: Auto-slide every 4 seconds (uncomment below if you want it)
        // setInterval(() => updateCarousel(currentSlide + 1), 4000);
    }
});