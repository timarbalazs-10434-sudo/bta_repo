import { Component, OnInit, ChangeDetectorRef } from '@angular/core'; // + ChangeDetectorRef
import { Router } from '@angular/router';
import { HttpClient } from '@angular/common/http';

@Component({
  selector: 'app-profile',
  standalone: true,
  templateUrl: './profile.html' // vagy a korábbi template string
})
export class Profile implements OnInit {
  user: any = null;

  constructor(
    private router: Router,
    private cdr: ChangeDetectorRef, // Injektáljuk be
    private http: HttpClient // Injektáljuk be a HttpClientet
  ) {}

  ngOnInit() {
    this.loadUser();
  }

  loadUser() {
    const data = localStorage.getItem('epic_user');
    if (data) {
      this.user = JSON.parse(data);
      // Manuálisan szólunk, hogy rajzolja újra az oldalt az adatokkal
      this.cdr.detectChanges(); 
      this.savePlayerToDb(); // Hívjuk meg az adatbázis mentő függvényt
    } else {
      this.router.navigate(['/']);
    }

    console.log(this.user);
  }

  logout() {
    localStorage.removeItem('epic_user');
    this.router.navigate(['/']);
  }

  goToLogin() {
    this.router.navigate(['/']);
  }

  savePlayerToDb() {
    const data = localStorage.getItem('epic_user');
    if (data)
    {
      const storedUser = JSON.parse(data) as { username: string, accountId: string }; // Interfész definiálása
      console.log(storedUser.accountId);

      const url = 'http://localhost:4000/api/player/login'; // Backend API endpoint
      const body = {
        username: storedUser.username,
        epicAccountId: storedUser.accountId
      };

      console.log('Küldött adatok:', body);
      this.http.post(url, body).subscribe({
        next: (response) => {
          console.log('Sikeres mentés az adatbázisba:', response);
        },
        error: (error) => {
          console.error('Hiba az adatbázis mentése során:', error);
        }
      });
    }
      

    
  }
}