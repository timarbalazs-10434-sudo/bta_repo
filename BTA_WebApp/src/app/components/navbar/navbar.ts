import { Component, inject, OnDestroy, OnInit} from '@angular/core';
import {  ActivatedRoute, RouterLink, RouterLinkActive } from "@angular/router";
import { InventoryService } from '../../services/inventory-service';

@Component({
  selector: 'app-navbar',
  imports: [RouterLink, RouterLinkActive],
  templateUrl: './navbar.html',
  styleUrl: './navbar.css',
})
export class Navbar implements OnInit, OnDestroy{
  menuimage:string = "menu_icons/menu.png";
  hiddenMenu:string = "hidden";
  onMenuClick(){
    if(this.menuimage == "menu_icons/menu.png")
    {
      this.menuimage = "menu_icons/menu_x.png"
      this.hiddenMenu = "visible"
    }
    else{
      this.menuimage = "menu_icons/menu.png"
      this.hiddenMenu = "hidden"
    }
  }

  client_id = "xyza7891tWfJEAorLEh2mGjYcBVbuari"
  loginEpicGames(){
    if (typeof window !== 'undefined') {
      window.open(`https://www.epicgames.com/id/authorize?response_type=code&client_id=${this.client_id}&redirect_uri=${encodeURIComponent('https://btaweb.vercel.app')}`, `EpicLogin`, `width=500, height=600`)
    }
  }

  displayedEpicCode: string | null = null;

  private messageListener: any;

  constructor(private route: ActivatedRoute) { }
  authService = inject(InventoryService);
  ngOnInit() {
    if (typeof window !== 'undefined') {
      this.messageListener = (event: MessageEvent) => {
        if (event.data && event.data.type === 'EPIC_CODE') {
          this.displayedEpicCode = event.data.code;
          this.authService.setCode(event.data.code);
          console.log('Kód frissítve:', this.displayedEpicCode);
        }
      };

      window.addEventListener('message', this.messageListener);
    }

    this.route.queryParams.subscribe(params => {
      const code = params['code'];
      if (code) {
        window.opener.postMessage({ type: 'EPIC_CODE', code: code }, '*');
        window.close();
      }
    });
  }

  ngOnDestroy() {
    if (typeof window !== 'undefined') {
      window.removeEventListener('message', this.messageListener);
    }
  }

  asd123(){
  }

}
