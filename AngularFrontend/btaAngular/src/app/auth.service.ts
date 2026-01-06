import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';

@Injectable({ providedIn: 'root' })
export class AuthService {
  private supabaseUrl = 'https://xavccwtdcdchavkivcmi.supabase.co/functions/v1/epic-auth';

  constructor(private http: HttpClient) {}

  // Ezt hívjátok meg, amikor a popup visszatér a kóddal
  completeEpicLogin(codeFromPopup: string) {
    return this.http.post(this.supabaseUrl, { code: codeFromPopup }, {
      headers: {
        'Content-Type': 'application/json',
        // 'Authorization': 'Bearer sb_publishable_AEaihSmmNCcD7k6QFUh-QA_3i3lACLD' // Ha be van állítva védelem
      }
    });
  }
}
