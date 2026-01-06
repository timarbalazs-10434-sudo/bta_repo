import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

@Component({
  selector: 'app-callback',
  standalone: true,
  template: '<h1>Bejelentkezés folyamatban...</h1>'
})
export class CallbackComponent implements OnInit {
  constructor(private route: ActivatedRoute) {}

  ngOnInit() {
    const code = this.route.snapshot.queryParamMap.get('code');
    if (code && window.opener) {
      // Küldjük az üzenetet a főablaknak
      window.opener.postMessage({ type: 'EPIC_LOGIN_SUCCESS', code: code }, window.location.origin);
      // Bezárjuk a popupot
      setTimeout(() => window.close(), 500);
    }
  }
}
