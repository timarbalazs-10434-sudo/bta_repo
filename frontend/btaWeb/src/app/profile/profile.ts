import { Component, OnInit, ChangeDetectorRef } from '@angular/core'; // + ChangeDetectorRef
import { Router } from '@angular/router';

@Component({
  selector: 'app-profile',
  standalone: true,
  templateUrl: './profile.html' // vagy a korábbi template string
})
export class Profile implements OnInit {
  user: any = null;

  constructor(
    private router: Router,
    private cdr: ChangeDetectorRef // Injektáljuk be
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
}