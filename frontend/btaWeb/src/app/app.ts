import { Component, ChangeDetectorRef, NgZone } from '@angular/core';
import { Router, RouterOutlet } from '@angular/router';
import { Profile } from "./profile/profile"; // Kell a RouterOutlet a HTML-be!

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, Profile], // Engedélyezzük az útvonalválasztót
  template: `
    <div style="text-align:center; margin-top: 50px; font-family: sans-serif;">
      <h1>Epic Games Login Teszt</h1>
      
      <button (click)="loginWithEpic()" style="padding: 10px 20px; cursor:pointer;">
        Bejelentkezés Epic-kel (Popup)
      </button>

      <hr style="margin: 20px 0;">
      
      <router-outlet></router-outlet>
    </div>
  `,
})
export class AppComponent {
  constructor(
    private cdr: ChangeDetectorRef,
    private zone: NgZone,
    private router: Router
  ) {}

  loginWithEpic() {
    const popup = window.open(
      'http://localhost:4000/auth/epic/login', 
      'EpicLogin', 
      'width=500,height=600'
    );

    const messageListener = (event: MessageEvent) => {
      if (event.origin !== 'http://localhost:4000') return;

      if (event.data.type === 'AUTH_SUCCESS') {
        // Nagyon fontos: Mindent a Zónán belül csinálunk!
        this.zone.run(() => {
          console.log('Backend küldte:', event.data.data);
          
          // 1. Mentés
          localStorage.setItem('epic_user', JSON.stringify(event.data.data));
          
          // 2. Navigáció
          this.router.navigate(['/profile']).then(nav => {
  console.log('Sikerült a navigáció?', nav); // Ha true, akkor a hiba a megjelenítésben van
}, err => {
  console.error('Navigációs hiba:', err); // Itt kiírja, ha nem találja az útvonalat

            this.cdr.detectChanges();
          });

          window.removeEventListener('message', messageListener);
        });
      }
    };

    window.addEventListener('message', messageListener);
  }
}