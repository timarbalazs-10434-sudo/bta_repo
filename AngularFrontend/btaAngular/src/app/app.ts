import { Component, signal, HostListener, OnInit } from '@angular/core';
import { Profile } from './profile/profile';
import { HttpClient, HttpHeaders } from '@angular/common/http';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [Profile],
  templateUrl: './app.html',
  styleUrl: './app.css'
})
export class App implements OnInit {
  protected readonly title = signal('btaAngular');

  // Sima objektumként definiálva
  userData: any;

  constructor(private http: HttpClient) {}

  @HostListener('window:message', ['$event'])
  onMessage(event: MessageEvent) {
    if (event.origin !== window.location.origin) return;

    if (event.data.type === 'EPIC_LOGIN_SUCCESS') {
      console.log('Kód megérkezett a popupból:', event.data.code);
      this.handleEpicLogin(event.data.code);
    }
  }

  loginWithEpic() {
    const clientId = "xyza78914PS090fc0UvUYkrEMtOYpsY0";
    const redirectUri = "http://localhost:4200/callback";

    const authUrl = "https://www.epicgames.com/id/authorize" +
          `?client_id=${clientId}` +
          "&response_type=code" +
          "&scope=basic_profile" +
          "&prompt=login" +
          `&redirect_uri=${encodeURIComponent(redirectUri)}`;

    window.open(authUrl, 'Epic Login', 'width=500,height=600');
  }

  async handleEpicLogin(code: string) {
    const supabaseUrl = 'https://xavccwtdcdchavkivcmi.supabase.co/functions/v1/epic-auth';
    const anonKey = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InhhdmNjd3RkY2RjaGF2a2l2Y21pIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njc1NDAzNzAsImV4cCI6MjA4MzExNjM3MH0.K1mzpoLUvTCBp9L-kp8EpYERcXyJM9qlnO6WYNxpEbA';

    const headers = new HttpHeaders({
      'Content-Type': 'application/json',
      'Authorization': `Bearer ${anonKey}`
    });

    console.log('Küldés a backendnek (Deno)...');

    this.http.post(supabaseUrl, { code: code }, { headers }).subscribe({
      next: (res: any) => {
        console.log('Sikeres válasz a Deno-tól:', res);
        // Sima objektumként mentjük el
        this.userData = res;
      },
      error: (err) => {
        console.error('Hiba a Deno hívás során:', err);
      }
    });
  }

  ngOnInit() {
    if (window.location.pathname === '/callback') {
      const code = new URLSearchParams(window.location.search).get('code');
      if (code && window.opener) {
        window.opener.postMessage({ type: 'EPIC_LOGIN_SUCCESS', code }, window.location.origin);
        window.close();
      }
    }
  }

  // JAVÍTVA: Itt levettem a ()-et, mert a userData már nem függvény (Signal)
  showUserInfo() {
    console.log(this.userData);
  }
}
