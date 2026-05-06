import { HttpClient, HttpHeaders } from '@angular/common/http';
import { effect, inject, Injectable, signal } from '@angular/core';
import { UserModel } from '../model/user-model.type';
import { InventoryModel } from '../model/inventory-model.type';

@Injectable({
  providedIn: 'root',
})
export class InventoryService {
  http = inject(HttpClient)
  token = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InhhdmNjd3RkY2RjaGF2a2l2Y21pIiwicm9sZSI6ImFub24iLCJpYXQiOjE3Njc1NDAzNzAsImV4cCI6MjA4MzExNjM3MH0.K1mzpoLUvTCBp9'

  headers = new HttpHeaders({
    Authorization : `Bearer ${this.token}`
  });

  //Body-ban tárol kód:
  epicCode = signal<string | null>(null);
  setCode(code: string) {
    this.epicCode.set(code);
    console.log('Bejött code:', code);
  }
  getBody() {
    return {
      code: this.epicCode()
    };
  }

  getUser(){
    const url = `https://xavccwtdcdchavkivcmi.supabase.co/functions/v1/epic-auth`;
    return this.http.post<Array<UserModel>>(url, this.getBody(), {
      headers: this.headers,
    });
  }

  userData = signal<any>(null);
  epicAccountId = signal<string | null>(null);
  username = signal<string | null>(null);

  setFullUser(userData: any) {
    this.userData.set(userData);
    const epicId = userData.EpicAccountId;
    const userName = userData.Username;
    if (epicId) {
      this.epicAccountId.set(epicId);
      this.username.set(userName);
    } else {
      console.log('Nincs Loginod.');
    }
  }

  getUserData() {
    return {
      EpicAccountId: this.epicAccountId(),
      Username: this.username()
    };
  }

  getInventory(){
    const url2 = `https://xavccwtdcdchavkivcmi.supabase.co/functions/v1/sessionInfo/${this.epicAccountId()}`
      return this.http.get<Array<InventoryModel>>(url2, {
        headers: this.headers,
    }) 
  }
}
