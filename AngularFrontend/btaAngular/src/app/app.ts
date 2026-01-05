import { Component, signal } from '@angular/core';
import { Profile } from './profile/profile';
import { createClient } from '@supabase/supabase-js';

@Component({
  selector: 'app-root',
  imports: [Profile],
  templateUrl: './app.html',
  styleUrl: './app.css'
})
export class App {
  protected readonly title = signal('btaAngular');



   loginWithEpic() {
  // Böngészőben NEM használunk Deno.env-et!
  const clientId = "xyza78914PS090fc0UvUYkrEMtOYpsY0";
  const redirectUri = "http://localhost:4200/callback";
  const scope = 'basic_profile friends_list';

  const authUrl = "https://www.epicgames.com/id/authorize" +
        `?client_id=${clientId}` +
        "&response_type=code" +
        "&scope=basic_profile" +
        "&prompt=login" +
        `&redirect_uri=${encodeURIComponent(redirectUri)}`;

  // Ez a böngészőben működik:
  window.open(authUrl, 'Epic Login', 'width=500,height=600');
}
}
